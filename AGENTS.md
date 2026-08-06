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

## 常见任务入口

| 任务 | 先看 | 高风险 |
| ---- | ---- | ---- |
| 新增或修改文件 API | `interfaces/kits/js/` + `utils/filemgmt_libn` | ● `*_core.cpp` 改动影响 NAPI+ANI 两条路径 |
| 新增 NAPI 导出 | `interfaces/kits/js/` + `utils/filemgmt_libn` | ● 公开 API 签名不可随意变更 |
| 修改 ANI/Static 绑定 | `interfaces/kits/js/` + ANI 生成目录 | ● 生成代码不要手改 |
| 修改 C API | `interfaces/kits/c/` + `interfaces/kits/native/` | ● C API 兼容性 |
| 修改错误处理 | `utils/filemgmt_libn/` + `utils/filemgmt_libfs/` | – |
| 修改 fd/资源管理 | `utils/filemgmt_libfs/` | ● fd 泄漏影响系统稳定性 |
| 新增测试 | `interfaces/test/unittest/` 或 `interfaces/test/fuzztest/` | – |

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

词汇触发：若任务描述、日志、issue 或代码中出现下列术语，先读对应专题页再动手：

| 术语 | 读取 |
| --- | --- |
| `NVal`、`NError`、`NClass`、`NFuncArg`、`NAsyncWork` | `docs/agents/napi-libn.md` |
| `FDGuard`、`FsResult`、`FsError` | `docs/agents/native-and-ffi.md` |
| `ANI`、`Taihe`、`Static TS`、`ETS`、`ABC` | `docs/agents/ani-static-binding.md` |
| `HyperAIO`、`file_api_feature_hyperaio` | `docs/agents/native-and-ffi.md` |
| `mod_fileio`、`mod_file`、`legacy` | `docs/agents/js-fs-api.md` |
| `UniError`、系统 errno 映射、`EACCES`、`EROFS`、`EINVAL` | `docs/agents/napi-libn.md` |
| `xattr`、`fdatasync`、`fsync`、`O_TRUNC`、`O_APPEND`、`copy/move`、`list` | `docs/agents/js-fs-api.md` |
| `securityLabel`、`selinux`、权限校验、URI 存储位置、`persist` | `docs/agents/feature-modules.md` |
| `RemoteURI`、`TaskSignal` | `docs/agents/native-and-ffi.md` |

编辑前声明：开始编辑前，先在回复中声明：

1. 任务类别：我正在修改 ______（API / NAPI / ANI / C API / 测试 / 其他）
2. 已阅读文档：我已读取 `docs/agents/______.md`
3. 遵循约束：我已确认 ______（NAPI/ANI 一致性 / fd 所有权 / 隐私 / 其他）

若无法确定任务类别或匹配的专题页，先提问，不要凭猜测直接改。

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
- 公共 API 兼容：`@ohos.file.fs`、`@ohos.file.hash`、`@ohos.file.environment`、
  `@ohos.file.statvfs`、`@ohos.file.securityLabel` 等是 SDK 公开 API。修改其签名、
  参数语义、错误码、Promise/Callback 行为或生命周期前，先确认是否需要兼容性审查，
  不要默认向后兼容可破坏。
- 权限与安全边界：`securityLabel`、URI 存储位置、沙箱路径校验、跨用户/跨账号文件
  访问属于权限/信任敏感逻辑。改动前先读 `docs/agents/feature-modules.md`，列出权限
  与信任影响；不要为让测试通过而绕过权限校验、调用方身份校验或路径校验。
- 生成文件 source-of-truth：ANI/Taihe 静态绑定、ETS/ABC 打包产物属于生成代码。
  不要直接编辑生成绑定；改 schema 或源定义后重新生成，再回到生成产物核对。
- 破坏性命令与设备操作：不要执行 `rm -rf`、`format`、批量删除用户文件、`truncate`
  到 0 等破坏性命令；涉及真机文件系统验证时先确认范围与可恢复性。
- 第三方依赖与 license：新增或升级第三方依赖前确认 license 兼容性与归属，不要
  临时引入未声明的依赖。
- 完整编译和运行测试依赖 OpenHarmony 源码树和产品构建环境。独立 Windows checkout
  只能做文本、diff 和结构验证，不能证明 OHOS 链接或设备运行行为。

## 改动前确认

以下变更必须先确认影响面再动手：

- 修改 `@ohos.file.*` 公开 API 签名或错误码 → 先评估兼容性影响
- 新增 BUILD.gn 依赖或 `bundle.json` 变更 → 先确认依赖方向
- 修改 `*_core.cpp` → 先确认 NAPI 和 ANI 两条路径是否都需要同步
- 删除或重命名公开接口 → 先确认无外部引用

## 反模式

- 不要手改 ANI 生成绑定代码 — 修改 `*_core.cpp` 或 IDL 后重新生成
- 不要在 NAPI execute 回调中抛 JS 异常 — 只在 complete 回调中处理
- 不要用 `malloc/free` 管理 fd 相关资源 — 用 `FDGuard` 确保异常路径也释放
- 不要在日志中打印完整文件路径或 URI — 隐私数据需脱敏

## 最小验证习惯

声明完成前，至少在仓库根目录运行结构检查：

```powershell
git diff -- AGENTS.md docs/agents
rg -n "TO[D]O|TB[D]|PLACE[H]OLDER" AGENTS.md docs/agents
rg -n "docs/agents/" AGENTS.md
```

涉及代码修改时，读取 `docs/agents/build-and-test.md`，选择覆盖改动区域的最小 GN
构建或单元测试目标。

最小构建/测试样例（命令与 target 以 `docs/agents/build-and-test.md` 为准）：

```bash
# 进入 OpenHarmony 源码根后，构建覆盖改动的最小 target
./build.sh --product-name <product> --build-target //foundation/filemanagement/file_api/interfaces/kits/js:fs
./build.sh --product-name <product> --build-target //foundation/filemanagement/file_api/interfaces/test/unittest:file_api_unittest
```

Done 定义：声明完成前须满足以下全部条件——

1. 结构检查通过（上方 `git diff` / `rg` 命令无遗留 TODO/占位符）。
2. 隐私日志启发式检查通过：
   `rg -n "HILOG[DIWE].*%\{public\}.*(path|uri|Uri|fileName|filename|bundle|realPath)" interfaces utils`
3. 覆盖改动区域的最小 GN target 构建成功，相关单测通过。
4. 任一项无法运行时，在最终回复中列出未验证项与原因，并给出下一个 OHOS 环境应运行的最小 target。

最终回复须包含：改动摘要、已运行验证（命令 + 环境：独立 Windows checkout 或完整
OHOS 源码树）、未运行验证及原因。无法运行完整构建时，明确说明并补上待运行的最小
target，不要默认"已验证"。
