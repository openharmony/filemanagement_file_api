# AGENTS.md - Unit Tests

本目录包含 file_api 模块的单元测试。

## 目录结构

```
unittest/
├── class_file/           # File class tests
├── common_mock/          # Mock utilities
├── common_utils/         # Test utilities
├── filemgmt_libn_test/   # NAPI library tests
├── hyperaio/             # Hyper async I/O tests
├── js/                   # JavaScript API tests
├── napi_js/              # NAPI JavaScript tests
├── napi_test/            # NAPI tests
├── remote_uri/           # Remote URI tests
├── resource/             # Test resources
└── task_signal/          # Task signal tests
```

## 特有约定

- 测试文件使用 cflags `-Dprivate=public -Dprotected=public` 访问私有成员
