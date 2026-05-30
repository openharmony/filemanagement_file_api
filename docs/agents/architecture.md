# 架构说明

当任务需要仓库背景、需求设计上下文或影响面分析时，读取本页。

## 领域角色

`file_api` 是 OpenHarmony 文件管理 API 组件。它提供应用侧和 inner-kit 文件能力：
文件和目录 I/O、流式读写、stat/statvfs、环境目录、hash、security label、压缩、
URI 处理、任务取消，以及可选的高性能异步 I/O。

本组件不是独立桌面库。它预期放在 OpenHarmony 源码树中构建：

```text
//foundation/filemanagement/file_api
```

这很重要，因为许多实现依赖 OpenHarmony 服务：ability runtime、bundle manager、
access token、data share、distributed file service、app file service、hilog、
hitrace、libuv、NAPI、ANI 和 system ability 基础设施。

## 分层模型

理解本仓库时，可以按以下层次思考：

```text
ArkTS/JavaScript public API
        |
        | dynamic binding
        v
NAPI wrapper layer
        |
        | static binding
        v
ANI / Taihe / generated ETS+ABC layer
        |
        v
Core filesystem semantics and entity classes
        |
        v
libuv, POSIX/system calls, OpenHarmony services, Rust/native helpers
```

不是每个 API 都经过所有层。legacy 模块大多以 NAPI 为主；现代 `@ohos.file.fs`
同时具有动态 NAPI 入口和静态 ANI/Taihe core 路径。

## 主要构建 group

`bundle.json` 声明了组件公开构建 group：

- `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_api`
- `//foundation/filemanagement/file_api/interfaces/kits/js:build_kits_js`
- `//foundation/filemanagement/file_api/interfaces/kits/ts/streamrw:streamrw_packages`
- `//foundation/filemanagement/file_api/interfaces/kits/ts/streamhash:streamhash_packages`
- `//foundation/filemanagement/file_api/interfaces/kits/cj:fs_ffi_packages`
- `//foundation/filemanagement/file_api/interfaces/kits/hyperaio:group_hyperaio`

它也声明了 inner kit，例如 `remote_uri_native`、`environment_native`、
`fileio_native`、`rust_file`、`filemgmt_libfs`、`filemgmt_libn`、
`filemgmt_libhilog`、C API library、CJ FFI library 和 `HyperAio`。

## Feature Flag

feature flag 位于 `file_api.gni`：

- `file_api_read_optimize`：读优化开关。
- `file_api_feature_hyperaio`：启用 HyperAIO 构建/测试路径和 liburing 集成。
- `file_api_hiviewdfx_api_metrics_enable`：当全局构建包含 api metrics 部件时自动启用。

设计改动时，要确认行为是否需要覆盖 mini、small、standard、Windows/Mingw、Mac，
还是只面向正常 OHOS 设备构建。

## 现代与 Legacy API 区域

现代文件 API 工作通常从这里开始：

```text
interfaces/kits/js/src/mod_fs
```

legacy 或兼容模块包括：

- `interfaces/kits/js/src/mod_fileio`：历史 `@ohos.fileio`。
- `interfaces/kits/js/src/mod_file`：历史 `@system.file`。
- `interfaces/kits/js/src/mod_statfs`：较旧的 statfs 风格模块。

除非需求明确要求跨模块兼容，否则不要为了现代 `@ohos.file.fs` 需求顺手重构 legacy 模块。

## 影响面分析清单

实现前先回答这些问题：

- 改动影响哪个 API 面：NAPI 动态 JS、ANI 静态 TS、C、CJ、native、Rust，还是仅测试？
- 目标行为是在共享 `*_core.cpp` 中实现，还是只在 wrapper 中实现？
- 同一个操作是否有 NAPI、ANI、CJ、C 变体需要保持一致？
- 操作接收的是 path、URI、fd、callback、listener，还是 task signal？
- 错误需要 errno 映射、OpenHarmony 特定错误映射，还是 callback/promise 转换？
- 操作是否涉及隐私日志、fd、mmap、remote URI、DataShare 或 distributed file service？
- 哪个 GN target 是覆盖该改动的最小有效构建或测试目标？

## 依赖方向

优先保持依赖向下流动：

- wrapper 负责解析输入、绑定 class、转换错误。
- core 文件负责可复用语义，返回 `FsResult` 或 errno 风格结果。
- entity class 持有运行期状态，例如 fd guard、path、URI、watcher、stream、iterator。
- utility library 承载通用 NAPI、文件系统错误和日志 helper。

除非确实是导出时初始化行为，否则不要把业务逻辑加到 module registration 文件里。
