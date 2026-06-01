# 构建与测试命令

选择最小验证方式时读取本页。

## 环境边界

本仓库是 OpenHarmony 组件。完整编译和运行验证需要 OpenHarmony 源码树，并且本仓库位于：

```text
foundation/filemanagement/file_api
```

当前独立 Windows checkout 可以验证文本、文件结构、链接和 diff，但不能单独证明 OHOS GN
链接或设备运行行为。

## 最小文本验证

在本仓库根目录运行：

```powershell
git status --short
git diff -- AGENTS.md docs/agents
rg -n "TO[D]O|TB[D]|PLACE[H]OLDER" AGENTS.md docs/agents
rg -n "docs/agents/" AGENTS.md
```

代码改动还应搜索是否误用 public 日志打印隐私数据：

```powershell
rg -n "HILOG[DIWE].*%\\{public\\}.*(path|uri|Uri|fileName|filename|bundle|realPath)" interfaces utils
```

这是启发式检查，不能替代码审查。

## 构建入口

组件级构建契约以 `bundle.json` 为准。重要 group：

```text
//foundation/filemanagement/file_api/interfaces/kits/js:build_kits_js
//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_api
//foundation/filemanagement/file_api/interfaces/kits/ts/streamrw:streamrw_packages
//foundation/filemanagement/file_api/interfaces/kits/ts/streamhash:streamhash_packages
//foundation/filemanagement/file_api/interfaces/kits/cj:fs_ffi_packages
//foundation/filemanagement/file_api/interfaces/kits/hyperaio:group_hyperaio
```

重要测试 group：

```text
//foundation/filemanagement/file_api/interfaces/test/unittest:file_api_unittest
//foundation/filemanagement/file_api/interfaces/test/fuzztest:file_api_fuzztest
```

## 常见 OHOS 构建形式

具体 product name 取决于工作区。进入 OpenHarmony 源码根后，先使用该工作区惯用的产品选择，
再构建覆盖改动的最小 target。

常见形式：

```bash
./build.sh --product-name <product> --build-target //foundation/filemanagement/file_api/interfaces/kits/js:fs
./build.sh --product-name <product> --build-target //foundation/filemanagement/file_api/interfaces/kits/js:ani_file_api
./build.sh --product-name <product> --build-target //foundation/filemanagement/file_api/interfaces/test/unittest:file_api_unittest
```

如果工作区使用 `hb`，等价模式是：

```bash
hb set
hb build --target //foundation/filemanagement/file_api/interfaces/kits/js:fs
hb build --target //foundation/filemanagement/file_api/interfaces/test/unittest:file_api_unittest
```

优先使用当前 OHOS workspace 已标准化的构建前端。

## 小 target 选择

选择最小有意义 target：

- `@ohos.file.fs` NAPI 改动：
  `//foundation/filemanagement/file_api/interfaces/kits/js:fs`
- `@ohos.file.fs` ANI/static 改动：
  `//foundation/filemanagement/file_api/interfaces/kits/js:file_fs_taihe` 或
  `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_api`
- Hash 动态模块：
  `//foundation/filemanagement/file_api/interfaces/kits/js:hash`
- Hash ANI 模块：
  `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_hash`
- Environment：
  `//foundation/filemanagement/file_api/interfaces/kits/js:environment` 或
  `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_environment`
- Statvfs：
  `//foundation/filemanagement/file_api/interfaces/kits/js:statvfs` 或
  `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_statvfs`
- Security label：
  `//foundation/filemanagement/file_api/interfaces/kits/js:securitylabel` 或
  `//foundation/filemanagement/file_api/interfaces/kits/js:ani_file_securitylabel`
- C fileio：
  `//foundation/filemanagement/file_api/interfaces/kits/c/fileio:ohfileio`
- C environment：
  `//foundation/filemanagement/file_api/interfaces/kits/c/environment:ohenvironment`
- C compression：
  `//foundation/filemanagement/file_api/interfaces/kits/c/compress:ohcompress`
- CJ FFI：
  `//foundation/filemanagement/file_api/interfaces/kits/cj:fs_ffi_packages`
- Native inner kit：
  `//foundation/filemanagement/file_api/interfaces/kits/native:build_kits_native`
- Rust helper：
  `//foundation/filemanagement/file_api/interfaces/kits/rust:rust_file`
- HyperAIO：
  `//foundation/filemanagement/file_api/interfaces/kits/hyperaio:group_hyperaio`

## 小测试 target 选择

`interfaces/test/unittest` 下的 target：

```text
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_fs_test
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_fs_mock_test
//foundation/filemanagement/file_api/interfaces/test/unittest/napi_js:napi_file_fs_mock_test
//foundation/filemanagement/file_api/interfaces/test/unittest/filemgmt_libn_test:filemgmt_libn_test
//foundation/filemanagement/file_api/interfaces/test/unittest/remote_uri:remote_uri_test
//foundation/filemanagement/file_api/interfaces/test/unittest/task_signal:task_signal_test
//foundation/filemanagement/file_api/interfaces/test/unittest/compress:compress_test
```

功能模块：

```text
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_hash_test
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_environment_test
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_securitylabel_test
//foundation/filemanagement/file_api/interfaces/test/unittest/js:ani_file_statvfs_test
//foundation/filemanagement/file_api/interfaces/test/unittest/napi_js:napi_file_environment_test
```

HyperAIO 测试受 feature flag 控制：

```text
//foundation/filemanagement/file_api/interfaces/test/unittest/hyperaio:hyperaio_test
//foundation/filemanagement/file_api/interfaces/test/fuzztest/hyperaio_fuzzer:HyperaioFuzzTest
```

## 验证报告要求

报告验证时要明确：

- 运行了哪些命令。
- 环境是独立 Windows checkout 还是完整 OHOS 源码树。
- 如果无法运行完整构建，明确说明，并给出下一个 OHOS 环境应运行的最小 target。
