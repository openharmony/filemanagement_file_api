# ANI 与静态绑定说明

修改 ANI wrapper、Static TS 行为、Taihe 生成绑定、ETS/ABC 打包或 NAPI/ANI 一致性时
读取本页。

## 本仓库中的 ANI 含义

ANI 是 ArkTS Native Interface 集成使用的静态绑定路径。相比动态 NAPI，ANI 代码通常：

- 使用生成或静态绑定胶水。
- 调用共享 `*_core.cpp` 实现。
- 通过 ANI helper 类型返回或抛错，而不是直接处理 NAPI value。
- 将 static ABC artifact 安装到 `/system/framework`。

## 主要位置

`@ohos.file.fs`：

- `interfaces/kits/js/src/mod_fs/properties/ani`：操作函数的 ANI wrapper。
- `interfaces/kits/js/src/mod_fs/class_*/ani`：class 的 ANI wrapper。
- `interfaces/kits/js/src/mod_fs/taihe/idl/ohos.file.fs.taihe`：Taihe IDL。
- `interfaces/kits/js/src/mod_fs/taihe/src`：Taihe 实现胶水。
- `interfaces/kits/js/src/mod_fs/properties/*_core.cpp`：共享语义。

功能模块：

- `src/mod_hash/ani`
- `src/mod_securitylabel/ani`
- `src/mod_environment/ani`
- `src/mod_statvfs/ani`

通用 ANI helper：

```text
interfaces/kits/js/src/common/ani_helper
```

## 构建目标

`interfaces/kits/js/BUILD.gn` 中重要 static/ANI target：

- `file_fs_taihe`：静态 `@ohos.file.fs` 实现，带 `shlib_type = "ani"`。
- `ohos_file_fs_abc` 和 `ohos_file_fs_abc_etc`：生成 static ABC 包。
- `ani_file_hash`、`ani_file_securitylabel`、`ani_file_environment`、
  `ani_file_statvfs`。
- `ani_file_api`：ANI 模块聚合 group。

当 API 在 `ani/ets` 下有 ETS 文件，或使用 Taihe 生成 ETS 时，确认符号名和生成 ABC target
仍与模块命名匹配。

## Core 优先

语义改动优先放到共享 core 文件：

```text
interfaces/kits/js/src/mod_fs/properties/*_core.cpp
interfaces/kits/js/src/mod_hash/*_core.cpp
interfaces/kits/js/src/mod_environment/*_core.cpp
interfaces/kits/js/src/mod_statvfs/*_core.cpp
interfaces/kits/js/src/mod_securitylabel/*_core.cpp
```

这样更容易保持 ANI 和 NAPI 行为一致。只有当需求涉及参数形态、语言绑定行为或静态绑定
签名时，才优先修改 wrapper。

## 一致性检查

修改某个操作时：

1. 同时搜索 NAPI wrapper、ANI wrapper 和 core 实现。
2. 检查 `unittest/js` 和 `unittest/napi_js` 是否都有相关测试。
3. 保持错误码、边界条件和资源所有权一致。
4. 如果某个绑定有意不同，在 code review 摘要或本地文档中说明原因。

有用搜索：

```powershell
rg -n "OpenCore|DoOpen|open_ani|Open::|FsResult" interfaces/kits/js/src/mod_fs
rg -n "bind_function_class|generate_static_abc|shlib_type = \"ani\"" interfaces/kits/js
```

## 测试

ANI/core 测试主要在：

```text
interfaces/test/unittest/js
```

重要 target：

- `ani_file_fs_mock_test`
- `ani_file_fs_test`
- `ani_file_hash_test`
- `ani_file_securitylabel_test`
- `ani_file_environment_test`
- `ani_file_statvfs_test`

mock 测试偏失败路径和平台/service 交互；非 mock 测试偏 core 行为和工具行为。

## 风险区域

- Taihe 生成输出会让改动看起来比较间接，需要同时检查 IDL 和贴近生成的实现代码。
- static ABC 打包必须保持 device destination path 稳定。
- ANI helper 和 type converter 集中管理签名和转换行为，不要在每个 wrapper 中重复实现转换规则。
- 如果 NAPI wrapper 额外处理 URI、DataShare 或 fd，先确认 core 路径是否已包含这些逻辑，
  再判断 ANI 是否获得同样行为。
