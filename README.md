解决一些插件在 `Windows`下的性能问题。

**只支持 Windows ！**

- 已实现的插件

- [X] userdb_sync_delete: 灵感来自[rime_wanxiang: userdb_sync_delete](https://github.com/amzxyz/rime_wanxiang/blob/main/lua/userdb_sync_delete.lua)

### 使用方法

1. 克隆项目到 librime 插件目录：
```
cd librime/plugins

git clone https://github.com/sheldonleung/rime-custom-plugins.git

重命名 rime-custom-plugins 为 custom-plugins
```

2. 编译：
```
build.bat
```

3. 配置:

在xxx.custom.yaml中添加配置以确保插件能够被 rime 加载，如：

```
engine/processors/@before 1: userdb_sync_delete
```

> 只面向有动手能力的小伙伴，librime 的具体编译过程请阅读 [librime](https://github.com/rime/librime/blob/master/README-windows.md) 官方教程，或结合官方 [CI](https://github.com/rime/librime/actions) 自行编译。
