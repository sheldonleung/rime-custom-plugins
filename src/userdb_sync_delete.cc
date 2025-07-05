#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime_api.h>

#if defined(_WIN32) || defined(_WIN64)
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <vector>
#endif

#include "lib/detached_thread_manager.hpp"
#include "userdb_sync_delete.hpp"

namespace fs = std::filesystem;

namespace rime {

/**
 * 获取目录下所有的 .userdb 文件夹
 */
std::vector<fs::path> get_userdb_folders(const fs::path& dir) {
  std::vector<fs::path> result;
  if (!fs::exists(dir)) {
    LOG(INFO) << "Doesn't has .userdb folderes.";
    return result;
  }
  if (!fs::is_directory(dir)) {
    return result;
  }
  for (const auto& entry : fs::directory_iterator(dir)) {
    try {
      if (entry.is_directory()) {
        const auto& path = entry.path();
        const std::string folder_name = path.filename().string();
        // 匹配以 .userdb 结尾的文件夹
        const std::string suffix = ".userdb";
        const size_t suffix_len = suffix.length();
        const size_t name_len = folder_name.length();
        if (name_len > suffix_len &&
            folder_name.substr(name_len - suffix_len) == suffix) {
          result.push_back(path);
        }
      }
    } catch (const fs::filesystem_error& e) {
      LOG(ERROR) << "Fail to get .usredb folders. ERROR MESSAGE: " << e.what();
    }
  }
  return result;
}

/**
 * 清理用户目录下的 .userdb 文件夹
 */
void clean_userdb_folders() {
  auto user_data_dir = rime_get_api()->get_user_data_dir();
  auto folders = get_userdb_folders(user_data_dir);
  if (!folders.empty()) {
    for (const auto& folder : folders) {
      for (const auto& entry : fs::directory_iterator(folder)) {
        try {
          fs::remove(entry.path());
        } catch (const fs::filesystem_error& e) {
          LOG(ERROR) << "Fail to delete '" << entry.path().string() << "'.";
        }
      }
    }
  }
}

/**
 * 获取 sync 目录下所有的 .userdb.txt 文件
 */
std::vector<fs::path> get_userdb_files() {
  std::vector<fs::path> result;
  std::string dir = rime_get_api()->get_user_data_dir();
  if (!fs::is_directory(dir) || !fs::exists(dir)) return result;
  std::string user_id;
#if defined(_WIN32) || defined(_WIN64)
  std::string inst_file = dir + "\\installation.yaml";
  std::string sync_dir = dir + "\\sync\\";
#else
  std::string inst_file = dir + "/installation.yaml";
  std::string sync_dir = dir + "/sync/";
#endif
  // rime_get_api()->get_user_id() 在小狼毫上获取的结果为unknow
  // 从 installation.yaml 中获取 user_id
  if (!fs::is_regular_file(inst_file)) return result;
  std::ifstream in(inst_file);
  if (!in.is_open()) return result;
  std::string line;
  line.reserve(256);
  std::string prefix = "installation_id: \"";
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    if (line.length() > prefix.length() && line.substr(0, prefix.length()) == prefix) {
      user_id = line.substr(prefix.length(), line.length() - prefix.length() - 1);
      break;
    }
    line.clear();
  }
  in.close();
  if (user_id.empty()) return result;
  sync_dir = sync_dir + user_id;

  for (const auto& entry : fs::directory_iterator(sync_dir)) {
    try {
      if (entry.is_regular_file()) {
        const auto& path = entry.path();
        const std::string file_name = path.filename().string();
        // 匹配以 .userdb.txt 结尾的文件
        const std::string suffix = ".userdb.txt";
        const size_t suffix_len = suffix.length();
        const size_t name_len = file_name.length();
        if (name_len > suffix_len &&
            file_name.substr(name_len - suffix_len) == suffix) {
          result.push_back(path);
        }
      }
    } catch (const fs::filesystem_error& e) {
      LOG(ERROR) << "Fail to get .userdb.txt files. ERROR MESSAGE: "
                 << e.what();
    }
  }
  return result;
}

/**
 * 从行中提取 c 值并解析
 */
double parse_c_value(const std::string& line) {
  // 从后往前查找"c="
  size_t pos = line.rfind("c=");
  if (pos == std::string::npos)
    return 1.0;  // 未找到 c 字段, 保留该行

  // 移动到c值起始位置 (跳过"c=")
  pos += 2;

  // 查找c值结束位置 (空格/制表符/行尾)
  size_t end = pos;
  while (end < line.size() &&
         !std::isspace(static_cast<unsigned char>(line[end]))) {
    end++;
  }

  double value = -1.0;
  auto [ptr, ec] = std::from_chars(line.data() + pos, line.data() + end, value);

  // 检查解析是否成功
  if (ec != std::errc() || ptr != line.data() + end) {
    try {
      return std::stod(line.substr(pos, end - pos));
    } catch (...) {
      return 1.0;  // 解析失败, 保留该行
    }
  }
  return value;
}

/**
 * 清理用户目录 sync 下的 .userdb 文件
 * @return 总共清理的无效词条数量
 */
int clean_userdb_files() {
  auto files = get_userdb_files();
  auto delete_item_count = 0;
  if (!files.empty()) {
    std::string line;
    line.reserve(256);

    for (const auto& file : files) {
      if (fs::exists(file) && fs::is_regular_file(file)) {
        std::ifstream in(file, std::ios::binary);
        std::string temp_file = file.string() + ".cache";
        std::ofstream out(temp_file, std::ios::binary);
        if (!in.is_open() || !out.is_open()) continue;

        while (std::getline(in, line)) {
          if (line.empty()) continue;
          // 提取并检查 c 值
          double c_value = parse_c_value(line);
          // 把 c > 0 的行写入新文件
          if (c_value > 0.0) {
            out << line << "\n";
          } else {
            delete_item_count++;
          }
          line.clear();
        }

        out.flush();
        out.close();
        in.close();

        fs::remove(file);
        std::string new_file = file.string();
        fs::rename(temp_file, new_file);
      }
    }
  }
  return delete_item_count;
}

/**
 * 发送清理结果通知
 */
void send_clean_msg(const int& deleteItemCount) {
#if defined(_WIN32) || defined(_WIN64)
  auto content = L"用户词典共清理  行无效词条";
  std::wstring str = content;
  std::wstringstream wss;
  wss << deleteItemCount;
  str.insert(8, wss.str());
  MessageBoxW(NULL, str.c_str(), L"通知", MB_OK);
#elif __APPLE__
#elif __linux__
#endif
}

/**
 * 执行清理任务
 */
void process_clean_task() {
  clean_userdb_folders();
  auto count = clean_userdb_files();
  send_clean_msg(count);
}

ProcessResult UserdbSyncDelete::ProcessKeyEvent(const KeyEvent& key_event) {
#if defined(_WIN32) || defined(_WIN64)
  auto ctx = engine_->context();
  auto input = ctx->input();
  if (input == "/del") {
    ctx->Clear();
    // 启动一个线程来执行清理任务, 避免系统等待用户关闭窗口导致系统阻塞
    DetachedThreadManager manager;
    if (manager.try_start(process_clean_task)) {
      return kAccepted;
    }
  }
#endif
  return kNoop;
}

}  // namespace rime
