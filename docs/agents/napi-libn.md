# NAPI 与 LibN 说明

修改 NAPI wrapper、JavaScript error 行为、Promise/Callback 调度、class export 或
`utils/filemgmt_libn` 时读取本页。

## LibN 的作用

`utils/filemgmt_libn` 是 NAPI 代码的共享 helper 层，用来抽象 NAPI 常见模式：

- `NVal`：包装 `napi_value`，并转换到 C++ 类型。
- `NFuncArg`：初始化和读取函数参数。
- `NError`：把 errno 风格失败映射到 JavaScript error。
- `NClass`：定义 class 并保存 native entity。
- `NExporter`：module/class/property export 的公共基类。
- `NAsyncWorkPromise` 和 `NAsyncWorkCallback`：异步执行模型。
- `NRef`：NAPI reference wrapper。

`interfaces/kits/js/src/common/napi` 下也有一份较新的 NAPI helper。修改前先确认目标
模块实际使用哪一套 helper。

## 常见 NAPI 流程

典型动态 API 流程：

1. 构造 `NFuncArg env/info`。
2. 用 `InitArgs(min, max)` 校验参数数量。
3. 用 `NVal` 转换参数。
4. 输入非法时调用 `NError(EINVAL).ThrowErr(env)`。
5. sync API 直接调用 core/system 操作并返回 JS value。
6. async API 创建 context/arg 对象，定义 execute 和 complete lambda，再调度 promise
   或 callback work。
7. 在 completion 阶段把执行错误转换为 JS error。

现有 operation 文件是最好的模板。`mod_fs/properties/open.cpp` 是 sync/promise/callback
分支的好参考。

## 错误规则

不要在 NAPI wrapper 中临时拼 JS error 对象。优先使用：

- `NError(errnoValue).ThrowErr(env)`：同步抛错路径。
- `NError(errnoValue).GetNapiErr(env)`：异步结果路径。
- 如果本地模块已经使用 `UniError` 等模块专用 wrapper，则沿用本地模式。

core 风格 API 应返回 `FsResult<T>::Error(code)` 或
`FsResult<T>::ErrorWithMsg(code, message)`，由 wrapper 负责转换。

当 libuv 或 POSIX 调用返回负 errno 风格值时，保持符号并遵循本地约定。现有代码经常把
负值传给 `NError` 或 `FsError`。

## Promise 与 Callback 分支

同时支持 Promise 和 Callback 的 API 通常遵循：

- 最后一个参数是 function 时选择 Callback。
- callback 缺失时选择 Promise。
- Callback 结果保持 error-first。
- Promise completion 的 resolve/reject 行为应与既有 API 兼容。

除非需求明确要求 JS API 兼容性变化，否则不要单独改变某个 API 的选择规则。

## Class Export 模式

class exporter 通常会：

- 提供 class name。
- 定义 constructor/static/member property。
- 把 native entity state 绑定到 JS 对象。
- 使用 `NClass::InstantiateClass` 创建实例。
- 使用 `NClass::GetEntityOf<T>` 获取 native state。

给 entity class 增加字段时，要同时检查 constructor、destructor、close 方法、异步
completion 和测试。entity state 常常持有 fd、path、URI、listener、ref 或 watcher cache。

## 日志与隐私

仓库使用 `HILOGD`、`HILOGI`、`HILOGW`、`HILOGE`。

经验规则：

- errno 值和固定 operation name 可以使用 `%{public}`。
- path、URI 字符串、文件名、bundle 派生用户数据、callback 输入应使用 `%{private}`，
  或不打印。
- 不要在热 read/write 循环里新增高频日志，除非已有 debug flag 保护。

`FileApiDebug::isLogEnabled` 和 `FileApiDebug::isTraceEnhanced` 会在支持的平台上由
`mod_fs/module.cpp` 从系统参数初始化。

## 平台宏

NAPI 代码常见平台宏：

- `WIN_PLATFORM`
- `IOS_PLATFORM`
- `CROSS_PLATFORM`
- `FILE_API_TRACE`
- `FILE_API_METRICS`

不要假设所有依赖都在所有构建中可用。DataShare、Ability Manager、Remote URI 和 fd
tagging 通常在 Windows/Mac host 风格构建中被排除。

## 测试路由

NAPI 相关测试主要在：

```text
interfaces/test/unittest/napi_js
interfaces/test/unittest/filemgmt_libn_test
interfaces/test/unittest/napi_test
```

NAPI wrapper 改动时，先在 `napi_js` 下按操作名搜索。如果改的是
`utils/filemgmt_libn`，使用 `filemgmt_libn_test`。
