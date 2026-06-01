# 功能模块说明

处理主 `@ohos.file.fs` 路径之外的功能，或任务明确提到某个模块时，读取本页。

## `mod_environment`

位置：

```text
interfaces/kits/js/src/mod_environment
```

作用：应用/环境目录 API。

相关 target：

- 动态 NAPI：`interfaces/kits/js:environment`
- ANI：`interfaces/kits/js:ani_file_environment`
- Native inner kit：`interfaces/kits/native:environment_native`
- C API：`interfaces/kits/c/environment:ohenvironment`

风险点：access token、os account、init parameter、bundle/app 目录规则、环境路径日志隐私。

## `mod_hash`

位置：

```text
interfaces/kits/js/src/mod_hash
```

作用：文件和流 hash。

相关 target：

- 动态 NAPI：`interfaces/kits/js:hash`
- ANI：`interfaces/kits/js:ani_file_hash`
- TS package：`interfaces/kits/ts/streamhash:streamhash_packages`

风险点：OpenSSL 使用、stream 生命周期、buffer 处理、一次性 hash 与 hash stream 行为一致性。

## `mod_statvfs` 与 `mod_statfs`

位置：

```text
interfaces/kits/js/src/mod_statvfs
interfaces/kits/js/src/mod_statfs
```

`mod_statvfs` 是现代 file statvfs 区域，具有动态和 ANI target。`mod_statfs` 是兼容风格模块。

相关 target：

- `interfaces/kits/js:statvfs`
- `interfaces/kits/js:ani_file_statvfs`
- `interfaces/kits/js:statfs`

风险点：path 校验、filesystem stat 结构、平台支持、错误映射。

## `mod_securitylabel`

位置：

```text
interfaces/kits/js/src/mod_securitylabel
```

作用：security label API。

相关 target：

- 动态 NAPI：`interfaces/kits/js:securitylabel`
- ANI：`interfaces/kits/js:ani_file_securitylabel`

风险点：label 值校验、权限行为、NAPI 与 ANI 一致性。

## `mod_fileio`

位置：

```text
interfaces/kits/js/src/mod_fileio
```

作用：legacy `@ohos.fileio` API 面。它仍作为 target `fileio` 构建，并安装到 `module`。

适用场景：

- 需求明确提到 `@ohos.fileio`。
- 兼容性测试失败。
- 需要保留 legacy app 行为。

除非有兼容性需求，否则不要把新的 `@ohos.file.fs` 行为放进 `mod_fileio`。

## `mod_file`

位置：

```text
interfaces/kits/js/src/mod_file
```

作用：legacy `@system.file` 模块。

把这里视为兼容代码。修改语义前检查 README 历史和测试。

## `mod_document`

位置：

```text
interfaces/kits/js/src/mod_document
```

作用：document 相关文件 API 区。不要假设它完全遵循 `mod_fs` 模式，先检查
`interfaces/kits/js/BUILD.gn` 和本地 module export。

## 压缩

位置：

```text
interfaces/kits/c/compress
```

作用：C 压缩/归档 API，包含 zip reader/writer 逻辑和 archive inner API header。

测试：

```text
interfaces/test/unittest/compress
```

target 包括 `streamwrite_test`、`zip_writer_test`、`archive_reader_test` 和 group
`compress_test`。

风险点：归档格式兼容性、`interfaces/test/unittest/resource/compress_data` 下的资源文件、
C API 错误码。

## TS 扩展包

位置：

```text
interfaces/kits/ts/streamrw
interfaces/kits/ts/streamhash
```

每个包包含 TypeScript 源码、native module C++ 文件、构建脚本和 GN target。任务提到
stream read/write packaging 或 stream hash packaging，而不是 core `mod_fs` 行为时，读这里。

## 如何选择功能页

如果需求是 file open/read/write/list/copy/move/watch，从 `js-fs-api.md` 开始。

如果需求是 hash、environment、statvfs、securityLabel、compress、fileio、system.file
或 document，先读本页；target 归属不清楚时，再读 `code-map.md` 或本地 `BUILD.gn`。
