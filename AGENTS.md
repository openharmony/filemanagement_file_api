# AGENTS.md

本文件是 AI Agent 处理本仓库任务时的轻量入口。先读本文件，再按任务类型只加载匹配的详细文档页。

## 阅读策略

不要一开始就读取 `docs/agents/` 下的所有文件。

默认只读本文件。涉及需求设计或代码开发时，最多按需加载：

1. 如果影响范围不清楚，读取 `docs/agents/architecture.md` 或
   `docs/agents/code-map.md`。
2. 读取一个与任务领域匹配的专题页。
3. 规划验证时，读取 `docs/agents/testing.md` 或
   `docs/agents/build-and-test.md`。

本仓库内容较多，一次性加载全部背景知识会浪费上下文，也会降低后续实现的精度。

## 仓库定位

`filemanagement_file_api` 是 OpenHarmony 文件管理 API 组件。在 OpenHarmony 源码树中的位置是：

```text
//foundation/filemanagement/file_api
```

组件元信息：

- 子系统：`filemanagement`
- 部件：`file_api`
- Bundle：`@ohos/file_api`
- 主要能力面：文件、目录、流、stat、hash、environment、statvfs/statfs、
  security label、压缩、URI、异步文件 I/O。

主要实现语言是 C++，通过 NAPI 和 ANI 暴露到 ArkTS/JavaScript。本仓库同时包含
TypeScript 包装、C API、CJ FFI、native inner kit、Rust helper 和测试代码。

## 快速代码地图

- `bundle.json`：组件元数据、feature flag、公开构建 group、inner kit 和测试目标。
- `file_api.gni`：共享 GN 变量，例如 `file_api_path`、`src_path`、`utils_path`、
  `file_api_read_optimize`、`file_api_feature_hyperaio` 和 metrics 开关。
- `interfaces/kits/js`：JS 面主实现区，是 `@ohos.file.fs`、`@ohos.file.hash`、
  `@ohos.file.environment`、`@ohos.file.statvfs`、`@ohos.file.securityLabel`、
  legacy `fileio` 和 ANI 静态绑定的主要代码区。
- `interfaces/kits/ts`：`streamrw`、`streamhash` TypeScript 扩展包。
- `interfaces/kits/c`：environment、fileio、compress 的 C API。
- `interfaces/kits/cj`：file、stat、stream、watcher、task signal、xattr 等 CJ FFI。
- `interfaces/kits/native`：remote URI、task signal、environment、fileio native inner kit。
- `interfaces/kits/rust`：`rust_file` helper crate。
- `interfaces/kits/hyperaio`：可选 HyperAIO 实现，由 `file_api_feature_hyperaio` 控制。
- `utils/filemgmt_libn`：共享 NAPI helper 库，包含 `NVal`、`NError`、`NClass`、
  `NFuncArg`、`NAsyncWork*`。
- `utils/filemgmt_libfs`：共享文件系统结果和错误 helper，包含 `FsResult`、
  `FsError` 和 array buffer helper。
- `utils/filemgmt_libhilog`：日志封装。
- `interfaces/test/unittest`：C++/NAPI/ANI 单元测试和 mock。
- `interfaces/test/fuzztest`：fuzz 测试，目前主要围绕 HyperAIO feature。

## 知识路由

按任务类型决定下一步读取哪个文档：

| 任务或问题 | 读取 |
| --- | --- |
| 需要仓库背景、分层、产物影响面或需求设计上下文 | `docs/agents/architecture.md` |
| 需要按目录理解职责和关键入口文件 | `docs/agents/code-map.md` |
| 修改 `@ohos.file.fs`、文件操作、流、stat、watcher、xattr、copy/move/list/read/write | `docs/agents/js-fs-api.md` |
| 修改 NAPI 参数、导出、JS error、Promise/Callback 或 `utils/filemgmt_libn` | `docs/agents/napi-libn.md` |
| 修改 ANI、Static TS、Taihe 生成绑定、ETS/ABC 打包或 NAPI/ANI 一致性 | `docs/agents/ani-static-binding.md` |
| 修改 C API、CJ FFI、native inner kit、Rust、Remote URI、TaskSignal 或 HyperAIO | `docs/agents/native-and-ffi.md` |
| 修改 `environment`、`hash`、`statvfs`、`statfs`、`securityLabel`、`compress`、`document`、`fileio` 或 `file` 模块 | `docs/agents/feature-modules.md` |
| 新增或修改测试、mock、fixture，或选择测试目标 | `docs/agents/testing.md` |
| 构建、选择 GN target 或做最小验证 | `docs/agents/build-and-test.md` |

## 硬约束

- 新行为优先落在现代 `@ohos.file.fs` 路径。除非任务明确要求，否则将 `mod_fileio`
  和 `mod_file` 视为兼容/legacy 区域。
- 同一 API 语义同时暴露给 NAPI 和 ANI 时，要保持行为一致。修改 `*_core.cpp`
  往往会影响 ANI/Taihe 路径；只改 NAPI wrapper 通常只影响动态 JS 绑定。
- 新增 GN target 时保留 OpenHarmony 组件归属：
  `subsystem_name = "filemanagement"`，`part_name = "file_api"`。
- 系统 errno 风格失败应通过仓库内错误 helper 映射，例如 `NError`、`FsError`、
  `UniError`，不要临时拼装 JS error。
- 不要用 public 日志占位符打印隐私数据。路径、URI、用户可控文件名通常都是隐私数据。
- 遵守 `README_zh.md` 中的本地 I/O 约束：路径以 UTF-8/UTF-16 为预期，URI 有存储位置限制。
- 明确管理 fd 和异步资源。所有权常由 `FDGuard`、entity class 或 `FsResult` 表达；
  不要让 raw fd 在成功/失败路径中处于含混状态。
- 完整编译和运行测试依赖 OpenHarmony 源码树和产品构建环境。独立 Windows checkout
  只能做文本、diff 和结构验证，不能证明 OHOS 链接或设备运行行为。

## 最小验证习惯

声明完成前，至少在仓库根目录运行结构检查：

```powershell
git diff -- AGENTS.md docs/agents
rg -n "TO[D]O|TB[D]|PLACE[H]OLDER" AGENTS.md docs/agents
rg -n "docs/agents/" AGENTS.md
```

涉及代码修改时，读取 `docs/agents/build-and-test.md`，选择覆盖改动区域的最小 GN
构建或单元测试目标。
