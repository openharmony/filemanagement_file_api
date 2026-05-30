# 测试说明

新增或修改测试，或选择验证 target 时读取本页。

## 测试根目录

```text
interfaces/test
```

主要 group：

- `interfaces/test/unittest`：单元测试。
- `interfaces/test/fuzztest`：fuzz 测试。

聚合单元测试 target：

```text
//foundation/filemanagement/file_api/interfaces/test/unittest:file_api_unittest
```

聚合 fuzz target：

```text
//foundation/filemanagement/file_api/interfaces/test/fuzztest:file_api_fuzztest
```

## 单元测试布局

重要目录：

- `class_file`：Rust-backed class file 测试。
- `common_mock`：syscall 和 OpenHarmony service mock。
- `common_utils`：共享测试工具。
- `filemgmt_libn_test`：`utils/filemgmt_libn` 测试。
- `hyperaio`：HyperAIO 测试，仅当 `file_api_feature_hyperaio` 为 true 时启用。
- `js`：`mod_fs`、environment、hash、securityLabel、statvfs 的 ANI/core 测试。
- `napi_js`：`mod_fs` 和 environment 的动态 NAPI 测试。
- `napi_test`：面向 NAPI 行为的 JS 单元测试包。
- `remote_uri`：remote URI 测试。
- `resource`：测试资源和 `ohos_test.xml`。
- `task_signal`：native task signal 测试。
- `compress`：压缩测试。

## 主要单元测试 target

来自 `interfaces/test/unittest/BUILD.gn`：

- `class_file:class_file_test`
- `filemgmt_libn_test:filemgmt_libn_test`
- `js:ani_file_environment_test`
- `js:ani_file_fs_mock_test`
- `js:ani_file_fs_test`
- `js:ani_file_hash_test`
- `js:ani_file_securitylabel_test`
- `js:ani_file_statvfs_test`
- `napi_js:napi_file_environment_test`
- `napi_js:napi_file_fs_mock_test`
- `remote_uri:remote_uri_test`
- `task_signal:task_signal_test`
- `compress:compress_test`
- 当 `file_api_feature_hyperaio` 为 true 时：`hyperaio:hyperaio_test`

## NAPI 与 ANI 测试选择

修改以下内容时使用 `napi_js` 测试：

- NAPI 参数解析。
- NAPI class export。
- Promise/callback 分支。
- wrapper 代码中的 JS error object 转换。
- 动态 module registration/export 行为。

修改以下内容时使用 `js` ANI/core 测试：

- `*_core.cpp` 文件。
- ANI wrapper。
- core filesystem 语义。
- static binding 路径使用的 class 实现。

共享语义改动尽量同时运行或更新两类测试。

## Mock

`interfaces/test/unittest/common_mock` 包含以下 mock：

- access token 和 token id
- DataShare/distributed file service 相关调用
- eventfd、fdsan、inotify、mmap、poll、stdio、sys_file、sys_xattr、unistd
- os account、parameter、uv fs 和其他平台调用

mock 测试常用：

```text
private=public
protected=public
```

这是有意为之，用于让测试访问内部状态。除非测试 target 不再需要内部可见性，否则不要移除。

## 资源

测试资源位于：

```text
interfaces/test/unittest/resource
```

压缩资源位于：

```text
interfaces/test/unittest/resource/compress_data
```

新增二进制资源前优先复用已有 fixture。确实需要新 fixture 时，保持最小化，并在测试名称中
说明覆盖的行为。

## Fuzz 测试

fuzz 根目录：

```text
interfaces/test/fuzztest
```

当前 fuzz group 仅在 `file_api_feature_hyperaio` 启用时加入 HyperAIO fuzz：

```text
interfaces/test/fuzztest/hyperaio_fuzzer:HyperaioFuzzTest
```

新增 fuzz 覆盖时，检查 OpenHarmony fuzz test 约定，并按需要加 feature/platform guard。

## 测试命名习惯

新增测试前，按操作名搜索：

```powershell
rg -n "Open|open|Read|read|List|list|Watcher|xattr" interfaces/test/unittest
```

沿用本地命名和 target 放置方式。优先在已有 operation 测试附近加窄测试，不要新增过宽泛的
测试文件。
