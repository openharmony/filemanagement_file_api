# 代码地图

当需要在设计或编码前定位文件时，读取本页。

## 根目录文件

- `bundle.json`：组件定义、syscap、feature flag、依赖、构建 group、inner kit 和测试目标。
- `file_api.gni`：共享 GN 路径和 build arg。
- `README_zh.md`：当前中文功能概览和约束。
- `README.md`：较旧的英文概览；对 legacy `fileio` 背景有用，但 `@ohos.file.fs`
  相关内容以 `README_zh.md` 更当前。
- `AGENTS.md`：AI 轻量路由入口。

## `interfaces/kits/js`

这是主实现区。

关键构建文件：

```text
interfaces/kits/js/BUILD.gn
```

关键 target：

- `fileio`：legacy 动态模块，安装到 `module`。
- `fs`：现代动态 `@ohos.file.fs` 模块，安装到 `module/file`。
- `hash`：动态 `@ohos.file.hash` 模块。
- `file`：legacy `@system.file` 模块。
- `statfs`、`statvfs`、`environment`、`securitylabel`、`document`。
- `file_fs_taihe` 和 `ani_file_api`：静态/ANI 路径。

源码布局：

- `src/common`：共享 C++ helper、NAPI helper 副本、file helper、ANI helper、
  trace/debug helper 和通用错误处理。
- `src/mod_fs`：现代文件系统 API 实现。
- `src/mod_fileio`：legacy FileIO 实现。
- `src/mod_file`：legacy system file 实现。
- `src/mod_environment`：环境目录 API。
- `src/mod_hash`：hash API 和 hash stream class。
- `src/mod_securitylabel`：security label API。
- `src/mod_statfs`：statfs 兼容模块。
- `src/mod_statvfs`：statvfs 模块。
- `src/mod_document`：document 相关模块。

## `src/mod_fs`

这是现代文件开发的主战场。

入口和共享 helper：

- `module.cpp`：`file.fs` 的 NAPI module registration；导出常量、属性函数和 class。
- `common_func.*`：NAPI 侧共享 helper，用于 flags、mode、URI/path、fd 工具。
- `fs_utils.*`：core 侧 helper，供 ANI/static 路径使用。
- `fdtag_func.*`：支持设备构建中的 fd tagging helper。

类：

- `class_file`：`File` / `FsFile` entity 和导出逻辑。
- `class_stream`：stream class 实现。
- `class_stat`：stat class 实现。
- `class_watcher`：watcher class 和 watcher data cache。
- `class_randomaccessfile`：random access file class。
- `class_atomicfile`：atomic file class。
- `class_filemapping`：mmap/file mapping 支持。
- `class_readeriterator`：目录 reader iterator。
- `class_tasksignal`：任务取消 signal。

操作：

- `properties`：NAPI wrapper 文件和共享 core 文件。
- `properties/*_core.cpp`：可复用语义实现，ANI 测试和静态绑定路径大量使用。
- `properties/ani`：操作函数的 ANI wrapper。
- `properties/napi`：部分操作的 NAPI 专用 helper，例如 mmap 和扩展 list file。
- `properties/copy_listener`：copy 操作的进度 listener 支持。
- `properties/file_filter`：list API 的文件过滤支持。

生成/静态绑定区域：

- `taihe/idl/ohos.file.fs.taihe`：Taihe IDL 源。
- `taihe/src`：生成或贴近生成代码的实现胶水。
- `ani`：静态绑定 helper。

## `interfaces/kits/ts`

包含 TypeScript 扩展包：

- `streamrw`：stream read/write 包。
- `streamhash`：stream hash 包。
- `gen_obj.gni`：TS object 生成 helper。

每个包都有 `BUILD.gn`、`src/*.ts`、native module C++ 文件和 `build_ts_js.py`。

## `interfaces/kits/c`

公开 C API 和 NDK 元数据：

- `environment`：C environment API 和 `libenvironment.ndk.json`。
- `fileio`：C fileio API 和 `libfileio.ndk.json`。
- `compress`：C 压缩/归档 API，包含 framework 和 inner API header。
- `common/error_code.h`：共享 C 错误码定义。

## `interfaces/kits/cj`

Cangjie FFI 层。常见模式：

- `*_ffi.*`：导出的 FFI 边界。
- `*_impl.*`：FFI 边界背后的实现 class。
- 操作文件，例如 `copy_file.*`、`copy_dir.*`、`move_file.*`、`list_file.*`、
  `xattr.*`、`watcher_impl.*`、`task_signal_impl.*`。
- `uni_error.*`、`utils.*`、`macro.h`：CJ FFI 共享支撑。

## `interfaces/kits/native`

native inner kit：

- `remote_uri`：remote URI 检测和 fd 提取。
- `task_signal`：native task signal 和 listener。
- `environment`：native environment directory helper。
- `fileio`：native fileio helper。

这些 target 被 JS/C/CJ/ANI 路径和测试消费，并在 `bundle.json` 中作为 inner kit 暴露。

## `interfaces/kits/rust`

Rust helper crate：

- `Cargo.toml`：package `rust_file`，edition 2021，依赖 `libc`。
- `src/lib.rs`、`src/ffi.rs`、`src/adapter.rs`：Rust 实现和 FFI 桥。
- `include/rust_file.h`：C/C++ header 边界。

## `interfaces/kits/hyperaio`

可选高性能异步 I/O：

- `BUILD.gn`：target `HyperAio` 和 group `group_hyperaio`。
- `include/hyperaio.h`、`include/hyperaio_trace.h`：公开 header。
- `src/hyperaio.cpp`、`src/hyperaio_trace.cpp`：实现。

仅当 `file_api_feature_hyperaio` 启用时，该 target 才链接 `liburing` 并定义
`HYPERAIO_USE_LIBURING`。

## `utils`

- `utils/filemgmt_libn`：共享 NAPI wrapper。修改 NAPI value 转换、class export
  或 async scheduling 前先读这里。
- `utils/filemgmt_libfs`：`FsResult`、`FsError`、array buffer helper 和通用
  filesystem result/error utility。
- `utils/filemgmt_libhilog`：hilog 日志封装。
- `utils/common/include/file_utils.h`：通用 file utility header。

## `interfaces/test`

- `unittest`：单元测试，包含 NAPI wrapper、ANI/core、native/remote URI/task signal、
  compress 测试和 mock。
- `unittest/common_mock`：syscall 和 OpenHarmony service mock。
- `unittest/common_utils`：共享测试工具。
- `unittest/resource`：测试资源和配置。
- `fuzztest`：fuzz group 和 HyperAIO fuzz target。
