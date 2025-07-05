#ifndef DETACHED_THREAD_MANAGER_HPP_
#define DETACHED_THREAD_MANAGER_HPP_

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

// TODO: 优化
class DetachedThreadManager {
 public:
  // 尝试启动一个分离线程执行任务
  bool try_start(std::function<void()> task) {
    // 创建状态的本地副本（延长状态生命周期）
    auto local_state = state_;

    // 快速检查是否已在运行
    if (local_state->running.load(std::memory_order_acquire)) {
      return false;
    }

    // 加锁确保启动操作的原子性
    std::lock_guard<std::mutex> lock(local_state->mtx);

    // 再次检查（防止竞争条件）
    if (local_state->running.load(std::memory_order_relaxed)) {
      return false;
    }

    // 标记为运行状态
    local_state->running.store(true, std::memory_order_release);

    // 创建包裹任务（捕获状态副本）
    auto wrapped_task = [local_state, task]() mutable {
      // 确保任务完成时状态重置（RAII方式）
      struct Guard {
        std::shared_ptr<State> state;
        ~Guard() { state->running.store(false, std::memory_order_release); }
      } guard{local_state};

      try {
        // 执行任务
        task();
      } catch (...) {
        // 不做异常处理
      }
    };

    // 启动并分离线程
    std::thread(std::move(wrapped_task)).detach();
    return true;
  }

  // 检查当前是否有线程在运行
  bool is_running() const {
    return state_->running.load(std::memory_order_acquire);
  }

 private:
  // 内部状态结构
  struct State {
    std::atomic<bool> running{false};
    std::mutex mtx;
  };

  // 共享状态
  std::shared_ptr<State> state_ = std::make_shared<State>();
};

#endif
