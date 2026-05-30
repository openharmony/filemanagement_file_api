# Native、FFI、Rust 与 HyperAIO 说明

处理 C API、CJ FFI、native inner kit、Rust helper、Remote URI、TaskSignal 和
HyperAIO 时读取本页。

## Native Inner Kit

位置：

```text
interfaces/kits/native
```

`interfaces/kits/native/BUILD.gn` 中的 target：

- `remote_uri_native`：remote URI helper。
- `task_signal_native`：native task signal 和 listener。
- `environment_native`：native environment helper。
- `fileio_native`：native fileio helper。
- `build_kits_native`：聚合 group。

这些都是 inner kit，并在 `bundle.json` 中声明。它们可能被 JS、C、CJ 或测试消费。修改
公开 inner-kit 边界时，要保留 header path 和 target label 的稳定性。

## C API

位置：

```text
interfaces/kits/c
```

模块：

- `environment`：target `ohenvironment`，header `environment.h`，元数据
  `libenvironment.ndk.json`。
- `fileio`：target `ohfileio`，header `fileio.h`，元数据 `libfileio.ndk.json`。
- `compress`：target `ohcompress`，压缩/归档 framework 和 inner API header。
- `common/error_code.h`：共享错误定义。

修改 C API 时要注意 ABI 和 NDK 元数据。不要在未检查调用方和生成元数据的情况下修改导出
函数签名。

## CJ FFI

位置：

```text
interfaces/kits/cj
```

target：

- `cj_file_fs_ffi`
- `cj_statvfs_ffi`
- `fs_ffi_packages`

文件模式：

- `*_ffi.*`：导出给 CJ 的边界。
- `*_impl.*`：边界背后的实现对象。
- 操作文件：`copy_file`、`copy_dir`、`move_file`、`list_file`、`fdatasync`、
  `fsync`、`lseek`、`symlink`、`xattr`。
- 类式实现：`file_impl`、`stream_impl`、`stat_impl`、`statvfs_impl`、
  `randomAccessFile_impl`、`readerIterator_impl`、`watcher_impl`、
  `task_signal_impl`。
- 支撑文件：`uni_error`、`utils`、`macro`、`translistener`。

CJ 错误转换应与 `uni_error.*` 保持一致。与 JS/ANI 共享的操作，在修改语义前先对比
`mod_fs` core 文件。

## Rust Helper

位置：

```text
interfaces/kits/rust
```

crate 名为 `rust_file`，edition 2021，构建类型包括 `cdylib`、`staticlib` 和 `lib`。
它依赖 `libc`，并通过以下 header 暴露 C/C++ 边界：

```text
interfaces/kits/rust/include/rust_file.h
```

Rust 改动应保持 `ffi.rs` 和 C++/测试调用方的 C ABI 假设。不要把单独 `cargo` 验证当成
OpenHarmony GN target 链接成功的证明；有条件时应使用 GN target。

## HyperAIO

位置：

```text
interfaces/kits/hyperaio
```

target：

- `HyperAio`
- `group_hyperaio`

feature flag：

```text
file_api_feature_hyperaio
```

flag 启用时，HyperAIO 会添加 `liburing:liburing` 并定义 `HYPERAIO_USE_LIBURING`。
HyperAIO 的 fuzz 和单元测试路径也受该 flag 控制。

测试路径：

- `interfaces/test/unittest/hyperaio`
- `interfaces/test/fuzztest/hyperaio_fuzzer`

## Remote URI

Remote URI 行为用于 file open 和相关 URI 路径。相关文件：

- `interfaces/kits/native/remote_uri/remote_uri.*`
- `interfaces/test/unittest/remote_uri`
- `interfaces/test/unittest/resource/remote_uri_test.txt`

修改 URI 行为时，要在支持的 OHOS 构建上验证本地 `file://` 风格处理和 remote/DataShare
行为。

## TaskSignal

任务取消横跨 native 和 JS 层：

- Native：`interfaces/kits/native/task_signal`
- JS/NAPI：`interfaces/kits/js/src/mod_fs/class_tasksignal`
- CJ：`interfaces/kits/cj/src/task_signal_impl.*`
- 测试：`interfaces/test/unittest/task_signal` 和 `js/mod_fs/class_tasksignal`

保持 listener 生命周期、reference 所有权和取消状态清晰。没有测试时，不要随意改 raw pointer
或 callback 生命周期。
