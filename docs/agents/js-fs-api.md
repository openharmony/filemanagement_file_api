# 现代 `@ohos.file.fs` 说明

处理 `interfaces/kits/js/src/mod_fs` 相关任务时读取本页。

## 心智模型

`mod_fs` 是现代文件 API 实现。它暴露名为 `file.fs` 的动态 NAPI module，并与
static/ANI 路径共享一部分 core 语义。

主入口：

```text
interfaces/kits/js/src/mod_fs/module.cpp
```

`module.cpp` 初始化 `OpenMode`、`WhenceType`、`AccessModeType` 等常量，并导出
属性函数和 class。

## 导出结构

模块导出：

- 通过 `properties/prop_n_exporter.*` 导出属性函数。
- 导出 `File`、`Stat`、`Stream`、`Watcher`、`RandomAccessFile`、`AtomicFile`、
  `FileMapping`、`ReaderIterator`、`TaskSignal` 等 class。

部分 class 和操作会被 `WIN_PLATFORM`、`IOS_PLATFORM` 等平台宏排除。

## Wrapper 与 Core 文件

大多数操作族有两种形态：

- NAPI wrapper：`properties/open.cpp`、`properties/read_text.cpp`、
  `properties/listfile.cpp` 等。负责解析 NAPI 参数、选择 sync/promise/callback
  模型、实例化 JS 对象、抛出或返回 JS error。
- Core 实现：`properties/open_core.cpp`、`properties/read_core.cpp`、
  `properties/listfile_core.cpp` 等。负责可复用的文件系统语义，通常返回
  `FsResult<T>`。

如果行为变更应同时影响 ANI/static 路径，优先修改 core 文件。如果只是 JS 参数兼容性
或 callback/promise 选择规则，修改 NAPI wrapper。

## 异步编程模型

动态 NAPI API 通常支持：

- Sync：函数名以 `Sync` 结尾，直接抛 JS error。
- Promise：异步函数不传最后一个 callback 时返回 Promise。
- Callback：异步函数传最后一个 callback 时，以 error-first 风格调度 callback。

常用 helper：

- `NFuncArg`：参数数量和参数访问。
- `NVal`：NAPI value 转换和类型检查。
- `NAsyncWorkPromise`：Promise 调度。
- `NAsyncWorkCallback`：Callback 调度。
- `NError`：errno 到 JS error 的转换。

新增或修改异步 API 时，如果该 API 已支持三种模型，需要同时验证三种模型。

## 文件描述符所有权

`mod_fs` 经常用 entity class 和 guard 包装 fd：

- `src/common/file_helper/fd_guard.cpp`
- `class_file/file_entity.h`
- `class_file/fs_file.*`
- `class_stream/fs_stream.*`
- `class_randomaccessfile/fs_randomaccessfile.*`

每条路径都要明确所有权：

- 成功时，fd 只转移给 entity/guard 一次。
- 对象实例化失败时，关闭 fd。
- 异步完成时，先转换错误，再构造 JS 返回值。
- 不要返回处于半所有权状态的 fd。

## Path、URI 与 Remote Access

path-like 输入可能是普通 path，也可能是 URI。相关依赖包括：

- `app_file_service:fileuri_native`
- `data_share:datashare_common`
- `data_share:datashare_consumer`
- `dfs_service:distributed_file_daemon_kit_inner`
- `interfaces/kits/native/remote_uri`

`open` 是很好的参考：它处理普通 path、`file://`、content broker URI、DataShare URI、
media/docs authority 和 remote URI fd 提取。

隐私规则：path 和 URI 字符串都是用户数据。除固定枚举、errno、非敏感诊断值外，日志中
应使用 private 占位符。

## 操作族路由

常见改动按以下路径查找：

- open/close/fd duplication：`open*`、`close*`、`dup*`、`fdopen_stream*`。
- read/write：`read_core.*`、`write_core.*`、`read_text*`、`read_lines*`。
- 目录列举和过滤：`listfile*`、`listfile_ext*`、`file_filter`。
- copy/move：`copy*`、`copy_file*`、`copy_dir*`、`move*`、`movedir*`、
  `copy_listener`。
- 元数据：`stat*`、`lstat*`、`utimes*`、`truncate*`、`xattr*`。
- 目录/文件创建和删除：`mkdir*`、`mkdtemp*`、`rmdir*`、`unlink*`、`symlink*`。
- sync 和 seek：`fsync*`、`fdatasync*`、`lseek*`。
- watcher：`watcher*`、`class_watcher`。
- mapping：`mmap*`、`class_filemapping`。
- 任务取消：`class_tasksignal` 和 native `task_signal`。

## NAPI/ANI 一致性

许多 `mod_fs` 操作同时有动态 NAPI wrapper 和 ANI wrapper。语义改动时同时搜索：

```powershell
rg -n "OpenCore|DoOpen|open_core|open_ani|Open::" interfaces/kits/js/src/mod_fs
```

把 `open` 替换为目标操作名。如果 `interfaces/test/unittest/napi_js` 和
`interfaces/test/unittest/js` 都有相关测试，尽量同时更新或运行两类 target。

## 常用本地搜索

```powershell
rg -n "class .*Core|Do[A-Z]|FsResult|NAsyncWork|ThrowErr|GetNapiErr" interfaces/kits/js/src/mod_fs
rg -n "OpenMode|WhenceType|AccessModeType|LocationType" interfaces/kits/js/src/mod_fs
rg -n "HILOG[DIWE]|%\\{public\\}|%\\{private\\}" interfaces/kits/js/src/mod_fs
```
