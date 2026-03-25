# AGENTS.md

This file provides guidance to Agents when working with code in this repository.

## Overview

`file_api` 是 OpenHarmony 文件管理子系统，位于 `//foundation/filemanagement/file_api`，通过 NAPI/ANI 桥接提供文件 I/O 的 JavaScript/TypeScript API。

**System Location**: `//foundation/filemanagement/file_api`
**Subsystem**: `filemanagement`
**Part Name**: `file_api`

## Architecture

The codebase follows a layered architecture with two types of bridge layers between JavaScript and C++:

**Call Flow:**
```
JavaScript/ArkTS API
         ↓
┌─────────────────────────────────────┐
│  NAPI Bridge (Dynamic TS)           │  ← Runtime binding
│  or                                 │
│  ANI Bridge (Static TS)             │  ← Compile-time binding
├─────────────────────────────────────┤
│  File System Core Layer             │
├─────────────────────────────────────┤
│  libuv (Async I/O) / System Calls   │
└─────────────────────────────────────┘
```

### 1. NAPI Bridge Layer (`utils/filemgmt_libn/`) - Dynamic TS

NAPI (Node-API) bridge layer, also known as Dynamic TS, provides runtime JavaScript-C++ interoperability (`napi:ace_napi`):

- **NVal** (`n_val.h`) - NAPI value wrapper with type checking and conversion
- **NError** (`n_error.h`) - Error handling with errno mapping to JS errors
- **NClass** (`n_class.h`) - Class definition and registration utilities
- **NAsyncWork** (`n_async/`) - Async operation support (Promise/Callback)
- **NFuncArg** (`n_func_arg.h`) - Function argument parsing
- **NExporter** (`n_exporter.h`) - Module export utilities

### 2. ANI Bridge Layer - Static TS

ANI (ArkTS Native Interface) bridge layer, also known as Static TS, provides compile-time optimized bindings for performance-critical operations. ANI offers better performance through ahead-of-time compilation and type-safe interfaces.

**Class implementations:** File, Stream, Stat, Watcher, RandomAccessFile, AtomicFile, TaskSignal, ReaderIterator

**Property operations:** `open_ani.h`, `read_ani.h`, `write_ani.h`, `copy_ani.h`, `move_ani.h`, `mkdir_ani.h`, etc.

### 3. File System Core Layer (`interfaces/kits/js/src/mod_fs/`)

Modern file system API implementation:

- **Classes:** FsFile, FsStream, FsStat, FsWatcher
- **Properties:** `*_core.cpp` files for operations (open, read, write, copy, move, mkdir, etc.)

### 4. Utility Layer (`utils/`)

- **filemgmt_libfs** - Filesystem error handling and results
- **filemgmt_libhilog** - Logging macros (HILOGD, HILOGI, HILOGW, HILOGE), depends on `hilog:libhilog`

## Directory Structure

The codebase is organized into the following structure:

```
file_api/
├── interfaces/
│   ├── kits/              # Public APIs
│   │   ├── js/            # JavaScript/NAPI implementations
│   │   │   └── src/
│   │   │       ├── mod_fs/           # File system (@ohos.file.fs) - 推荐
│   │   │       ├── mod_fileio/       # Legacy FileIO (@ohos.fileio) - 已废弃
│   │   │       ├── mod_file/         # File module (@system.file) - 已废弃
│   │   │       ├── mod_environment/  # Environment (@ohos.file.environment)
│   │   │       ├── mod_hash/         # Hash (@ohos.file.hash)
│   │   │       ├── mod_statfs/       # Statfs (@ohos.statfs) - 已废弃
│   │   │       ├── mod_statvfs/      # Statvfs (@ohos.file.statvfs)
│   │   │       ├── mod_securitylabel/# Security label (@ohos.file.securityLabel)
│   │   │       └── common/           # Shared utilities
│   │   ├── ts/            # TypeScript APIs (streamrw, streamhash)
│   │   ├── c/             # C API (environment, fileio)
│   │   ├── cj/            # CJ language bindings
│   │   ├── native/        # Native C++ APIs
│   │   └── rust/          # Rust bindings
│   └── test/              # Test code
│       ├── unittest/      # Unit tests
│       └── fuzztest/      # Fuzz tests
├── utils/                 # Shared utilities
│   ├── filemgmt_libn/     # NAPI utilities (NVal, NError, NClass, NAsyncWork)
│   ├── filemgmt_libfs/    # Filesystem utilities (FsError, FsResult)
│   └── filemgmt_libhilog/ # Logging utilities
├── bundle.json            # Component configuration
└── file_api.gni           # GN build variables
```

## Key Subsystems

### Error Handling System

- **Location**: `utils/filemgmt_libn/include/n_error.h`
- **Purpose**: Maps system errno to JavaScript errors with proper error codes
- **Error Code Ranges**:
  - `FILEIO_SYS_CAP_TAG` (13900000) - FileIO errors
  - `USER_FILE_MANAGER_SYS_CAP_TAG` (14000000) - User file manager errors
  - `STORAGE_SERVICE_SYS_CAP_TAG` (13600000) - Storage service errors

### Async Operation System

- **Location**: `utils/filemgmt_libn/src/n_async/`
- **Purpose**: Handles async operations with Promise and Callback patterns
- **Components**:
  - `NAsyncWorkPromise` - Promise-based async operations
  - `NAsyncWorkCallback` - Callback-based async operations

## Code Conventions

OpenHarmony 特有约定：

- **命名空间**: `OHOS::FileManagement::<Module>`
- **公共方法**: PascalCase (`GetFD()`, `Constructor()`)
- **私有成员**: camelCase + trailing underscore (`env_`, `val_`)
- **日志**: 使用 HILOG 宏 (`HILOGD`, `HILOGI`, `HILOGW`, `HILOGE`)，隐私信息（用户路径、URI 等）禁止使用 `%{public}` 打印

## Important Notes

1. **No comments in code** unless explicitly requested
2. **UTF-8 paths**: Only UTF-8/16 encoding supported
3. **URI constraints**: URIs cannot include external storage directories


