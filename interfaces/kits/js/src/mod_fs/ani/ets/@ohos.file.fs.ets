/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { BusinessError, AsyncCallback } from '@ohos.base';
import stream from '@ohos.util.stream';

const UNKNOWN_ERR: int = 13900042
const UNKNOWN_MSG: string = "Unknown error"

function createBusinessError(code: int, msg: string): BusinessError {
  let err = new BusinessError();
  err.code = code;
  err.message = msg;
  return err;
}

class Cleaner {
    private ptr: long = 0
    private classType: string = "";
    constructor(ptr: long, classType: string) {
        this.ptr = ptr
        this.classType = classType
    }
    native clean(): void
}

function CleanerCallback(cleaner: Cleaner): void {
    cleaner.clean()
}

let destroyRegister = new FinalizationRegistry<Cleaner>(CleanerCallback)

namespace fileIo {
  export namespace OpenMode {
    export const READ_ONLY = 0o0;
    export const WRITE_ONLY = 0o1;
    export const READ_WRITE = 0o2;
    export const CREATE = 0o100;
    export const TRUNC = 0o1000;
    export const APPEND = 0o2000;
    export const NONBLOCK = 0o4000;
    export const DIR = 0o200000;
    export const NOFOLLOW = 0o400000;
    export const SYNC = 0o4010000;
  }

function access(path: string, mode?: AccessModeType): Promise<boolean> {
  return new Promise<boolean>((resolve: (result: boolean) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: string, mode?: AccessModeType): boolean => {
      return FileIoImpl.doAccessSync(path, mode);
    }, path, mode);
    promise.then((ret: NullishType): void => {
      let result = ret as boolean;
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function access(path: string, callback: AsyncCallback<boolean>): void {
  let promise = taskpool.execute((path: string): boolean => {
    return FileIoImpl.doAccessSync(path);
  }, path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as boolean;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, false);
  });
}

function access(path: string, mode: AccessModeType, flag: AccessFlagType): Promise<boolean> {
  return new Promise<boolean>((resolve: (result: boolean) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: string, mode: AccessModeType, flag: AccessFlagType): boolean => {
      return FileIoImpl.doAccessSync(path, mode, flag);
    }, path, mode, flag);
    promise.then((ret: NullishType): void => {
      let result = ret as boolean;
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  })
}

function accessSync(path: string, mode?: AccessModeType): boolean {
  return FileIoImpl.doAccessSync(path, mode);
}

function accessSync(path: string, mode: AccessModeType, flag: AccessFlagType): boolean {
  return FileIoImpl.doAccessSync(path, mode, flag);
}

function close(file: int | File): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((file: int | File): undefined => FileIoImpl.closeSync(file), file);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function close(file: int | File, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: int | File): undefined => FileIoImpl.closeSync(file), file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function closeSync(file: int | File): void {
  return FileIoImpl.closeSync(file)
}

function connectDfs(networkId: string, listeners: DfsListeners): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((networkId: string, listeners: DfsListeners): void =>
      FileIoImpl.connectDfs(networkId, listeners), networkId, listeners);
    promise.then<void>((): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function disconnectDfs(networkId: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((networkId: string): void => FileIoImpl.disConnectDfs(networkId), networkId);
    promise.then<void>((): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function getxattrSync(path: string, key: string): string {
  return FileIoImpl.getxattrSync(path, key)
}

function getxattr(path: string, key: string): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string, key: string): string => {
      return FileIoImpl.getxattrSync(path, key);
    }, path, key);
    promise.then((ret: NullishType): void => {
      let result = ret as string;
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function dup(fd: int): File {
  return FileIoImpl.dup(fd);
}

function copyDirSync(src: string, dest: string, mode?: int): void {
  return FileIoImpl.copyDirSync(src, dest, mode);
}

function copyDir(src: string, dest: string, mode?: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<Array<ConflictFiles>>) => void): void => {
    let promise = taskpool.execute((src: string, dest: string, mode?: int): undefined =>
      FileIoImpl.copyDirSync(src, dest, mode), src, dest, mode);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<Array<ConflictFiles>>);
    });
  });
}

function copyDir(src: string, dest: string, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string): undefined =>
    FileIoImpl.copyDirSync(src, dest), src, dest);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError<Array<ConflictFiles>>, undefined);
  });
}

function copyDir(src: string, dest: string, mode: int, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: int): undefined =>
    FileIoImpl.copyDirSync(src, dest, mode), src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError<Array<ConflictFiles>>, undefined);
  });
}

function copyDir(arg0: Object, arg1: Object, arg2: NullishType, arg3: NullishType): void {
  let src = arg0 as string;
  let dest = arg1 as string;
  if (typeof arg2 === 'int' && typeof arg3 === 'function') {
    let mode: int = 0;
    try {
      mode = arg2 as int;
    } catch (error) {
      mode = (arg2 as int) + 0;
    }
    let callback = arg3 as AsyncCallback<void, Array<ConflictFiles>>;
    copyDir(src, dest, mode, callback);
    return;
  }
  if (typeof arg2 === 'function') {
    let callback = arg2 as AsyncCallback<void, Array<ConflictFiles>>;
    copyDir(src, dest, callback);
    return;
  }
}

function mkdirSync(path: string): void {
  return FileIoImpl.mkdirSync(path)
}

function fdatasyncSync(fd: int): void {
  return FileIoImpl.fdatasyncSync(fd)
}

function fdatasync(fd: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((fd: int): undefined => FileIoImpl.fdatasyncSync(fd), fd);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function fdatasync(fd: int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((fd: int): undefined => FileIoImpl.fdatasyncSync(fd), fd);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function mkdirSync(path: string, recursion: boolean): void {
  return FileIoImpl.mkdirSync(path, recursion)
}

function mkdirSync1(path: string): undefined {
  FileIoImpl.mkdirSync(path);
  return undefined;
}

function mkdirSync2(path: string, recursion: boolean): undefined {
  FileIoImpl.mkdirSync(path, recursion);
  return undefined;
}

function mkdir(path: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string): undefined => mkdirSync1(path), path);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function mkdir(path: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string): undefined => mkdirSync1(path), path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function mkdir(path: string, recursion: boolean): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string, recursion: boolean): undefined =>
      mkdirSync2(path, recursion), path, recursion);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: NullishType): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function mkdir(path: string, recursion: boolean, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string, recursion: boolean): undefined =>
    mkdirSync2(path, recursion), path, recursion);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function moveDirSync(src: string, dest: string, mode?: int): void {
  return FileIoImpl.movedirSync(src, dest, mode)
}

function moveDir(src: string, dest: string, mode?: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<Array<ConflictFiles>>) => void): void => {
    let promise = taskpool.execute((src: string, dest: string, mode?: int): undefined => {
      return FileIoImpl.movedirSync(src, dest, mode);
    }, src, dest, mode);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<Array<ConflictFiles>>);
    });
  })
}

function moveDir(src: string, dest: string, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string): undefined => {
    return FileIoImpl.movedirSync(src, dest);
  }, src, dest);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError<Array<ConflictFiles>>, undefined);
  });
}

function moveDir(src: string, dest: string, mode: int, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: int): undefined => {
    return FileIoImpl.movedirSync(src, dest, mode);
  }, src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError<Array<ConflictFiles>>, undefined);
  });
}

function moveDir(arg0: Object, arg1: Object, arg2: NullishType, arg3: NullishType): void {
  let src = arg0 as string;
  let dest = arg1 as string;
  if (typeof arg2 === 'int' && typeof arg3 === 'function') {
    let mode: int = 0;
    try {
      mode = arg2 as int;
    } catch (error) {
      mode = (arg2 as int) + 0;
    }
    let callback = arg3 as AsyncCallback<void, Array<ConflictFiles>>;
    moveDir(src, dest, mode, callback);
    return;
  }
  if (typeof arg2 === 'function') {
    let callback = arg2 as AsyncCallback<void, Array<ConflictFiles>>;
    moveDir(src, dest, callback);
    return;
  }
}

function mkdtempSync(prefix: string): string {
  return FileIoImpl.mkdtempSync(prefix);
}

function mkdtemp(prefix: string): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((prefix: string): string => {
      return FileIoImpl.mkdtempSync(prefix);
    }, prefix);
    promise.then((ret: NullishType): void => {
      let result = ret as string;
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function mkdtemp(prefix: string, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute((prefix: string): string => {
    return FileIoImpl.mkdtempSync(prefix);
  }, prefix);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as string;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, "");
  });
}

function moveFileSync(src: string, dest: string, mode?: int): void {
  return FileIoImpl.moveFileSync(src, dest, mode);
}

function moveFile(src: string, dest: string, mode?: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((src: string, dest: string, mode?: int): undefined => {
      return FileIoImpl.moveFileSync(src, dest, mode);
    }, src, dest, mode);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function moveFile(src: string, dest: string, mode: int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: int): undefined => {
    return FileIoImpl.moveFileSync(src, dest, mode);
  }, src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function moveFile(src: string, dest: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string, dest: string): undefined => {
    return FileIoImpl.moveFileSync(src, dest);
  }, src, dest);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function openSync(path: string, mode?: int): File {
  return FileIoImpl.openSync(path, mode);
}

function open(path: String, mode?: int): Promise<File> {
  return new Promise<File>((resolve: (result: File) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: String, mode?: int): File => {
      return FileIoImpl.openSync(path, mode);
    },path, mode);
    promise.then((ret: NullishType): void => {
      let file = ret as File;
      resolve(file);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function open(path: String, mode: int, callback: AsyncCallback<File, void>): void {
  let promise = taskpool.execute((path: String, mode: int): File => {
    return FileIoImpl.openSync(path, mode);
  }, path, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let file = ret as File;
    callback(e, file);
  }).catch((e: Error): void => {
    let f: File = new FileInner(0);
    callback(e as BusinessError, f);
  });
}

function open(path: String, callback: AsyncCallback<File, void>): void {
  let promise = taskpool.execute((path: String): File => {
    return FileIoImpl.openSync(path);
  }, path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let file = ret as File;
    callback(e, file);
  }).catch((e: Error): void => {
    let f: File = new FileInner(0);
    callback(e as BusinessError, f);
  });
}

function writeSync(fd: int, buffer: string | ArrayBuffer, options?: WriteOptions): long {
  return FileIoImpl.writeSync(fd, buffer, options);
}

function write(fd: int, buffer: string | ArrayBuffer, options?: WriteOptions): Promise<long> {
  return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((fd: int, buffer: string | ArrayBuffer, options?: WriteOptions): long => {
      return FileIoImpl.writeSync(fd, buffer, options);
    }, fd, buffer, options);
    promise.then((ret: NullishType): void => {
      let result = ret as long
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function write(fd: int, buffer: string | ArrayBuffer, options: WriteOptions,
  callback: AsyncCallback<long, void>): void {
  let promise = taskpool.execute((fd: int, buffer: string | ArrayBuffer, options: WriteOptions): long => {
    return FileIoImpl.writeSync(fd, buffer, options);
  }, fd, buffer, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as long;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, 0);
  });
}

function write(fd: int, buffer: string | ArrayBuffer, callback: AsyncCallback<long, void>): void {
  let promise = taskpool.execute((fd: int, buffer: string | ArrayBuffer): long => {
    return FileIoImpl.writeSync(fd, buffer);
  }, fd, buffer);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as long;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, 0);
  });
}

function readSync(fd: int, buffer: ArrayBuffer, options?: ReadOptions): long {
  return FileIoImpl.readSync(fd, buffer, options)
}

function read(fd: int, buffer: ArrayBuffer, options?: ReadOptions): Promise<long> {
  return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((fd: int, buffer: ArrayBuffer, options?: ReadOptions): long => {
      return FileIoImpl.readSync(fd, buffer, options)
    }, fd, buffer, options);
    promise.then((ret: NullishType) => {
      let result = ret as long;
      resolve(result);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function read(fd: int, buffer: ArrayBuffer, callback: AsyncCallback<long, void>): void {
  let promise = taskpool.execute((fd: int, buffer: ArrayBuffer): long => {
    return FileIoImpl.readSync(fd, buffer);
  }, fd, buffer);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as long;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, 0);
  });
}

function read(fd: int, buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long, void>): void {
  let promise = taskpool.execute((fd: int, buffer: ArrayBuffer, options: ReadOptions): long => {
    return FileIoImpl.readSync(fd, buffer, options);
  }, fd, buffer, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as long;
    callback(e, result);
  }).catch((e: Error): void => {
    callback(e as BusinessError, 0);
  });
}

function readLinesSync(filePath: string, options?: Options): ReaderIterator {
  return FileIoImpl.readlinesSync(filePath, options)
}

function readLines(filePath: string, options?: Options): Promise<ReaderIterator> {
  return new Promise<ReaderIterator>((resolve: (result: ReaderIterator) => void,
    reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((filePath: string, options?: Options): ReaderIterator => {
      return FileIoImpl.readlinesSync(filePath, options);
    }, filePath, options);
    promise.then((ret: NullishType): void => {
      let it = ret as ReaderIterator;
      resolve(it);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function readLines(filePath: string, callback: AsyncCallback<ReaderIterator>): void {
  let promise = taskpool.execute((filePath: string): ReaderIterator => {
    return FileIoImpl.readlinesSync(filePath);
  }, filePath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let it = ret as ReaderIterator;
    callback(e, it);
  }).catch((e: Error): void => {
    let r: ReaderIterator = new ReaderIteratorInner(0);
    callback(e as BusinessError, r);
  });
}

function readLines(filePath: string, options: Options, callback: AsyncCallback<ReaderIterator>): void {
  let promise = taskpool.execute((filePath: string, options: Options): ReaderIterator => {
    return FileIoImpl.readlinesSync(filePath, options);
  }, filePath, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let it = ret as ReaderIterator;
    callback(e, it);
  }).catch((e: Error): void => {
    let r: ReaderIterator = new ReaderIteratorInner(0);
    callback(e as BusinessError, r);
  });
}

function rmdirSync(path: string): void {
  return FileIoImpl.rmdirSync(path)
}

function rmdir(path: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string): void => FileIoImpl.rmdirSync(path), path);
    promise.then((ret: NullishType) => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function rmdir(path: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string): void => FileIoImpl.rmdirSync(path), path);
  promise.then((ret: NullishType) => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function truncateSync(file: string | int, len?: long): void {
  return FileIoImpl.truncateSync(file, len)
}

function truncate(file: string | int, len?: long): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((file: string | int, len?: long): undefined => {
      return FileIoImpl.truncateSync(file, len);
    }, file, len);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  })
}

function truncate(file: string | int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: string | int): undefined => {
    return FileIoImpl.truncateSync(file);
  }, file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function truncate(file: string | int, len: long, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: string | int, len: long): undefined => {
    return FileIoImpl.truncateSync(file, len);
  }, file, len);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function unlinkSync(path: string): void {
  return FileIoImpl.unlinkSync(path)
}

function unlink(path: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string): undefined => unlinkSync(path), path);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function unlink(path: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string): undefined => unlinkSync(path), path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function readText(filePath: string, options?: ReadTextOptions): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((filePath: string, options?: ReadTextOptions): string => {
      return FileIoImpl.readTextSync(filePath, options);
    }, filePath, options);
    promise.then((ret: NullishType): void => {
      let r = ret as string;
      resolve(r);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function readText(filePath: string, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute((filePath: string): string => {
    return FileIoImpl.readTextSync(filePath);
  }, filePath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string;
    callback(e, r);
  }).catch((e: Error): void => {
    callback(e as BusinessError, "");
  });
}

function readText(filePath: string, options: ReadTextOptions, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute((filePath: string, options: ReadTextOptions): string => {
    return FileIoImpl.readTextSync(filePath, options);
  }, filePath, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string;
    callback(e, r);
  }).catch((e: Error): void => {
    callback(e as BusinessError, "");
  });
}

function readTextSync(filePath: string, options?: ReadTextOptions): string {
  return FileIoImpl.readTextSync(filePath, options);
}

function listFile(path: string, options?: ListFileOptions): Promise<string[]> {
  return new Promise<string[]>((resolve: (result: string[]) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: string, options?: ListFileOptions):  string[] => {
      return FileIoImpl.listFileSync(path, options);
    }, path, options);
    promise.then((ret: NullishType): void => {
      let r = ret as string[];
      resolve(r);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function listFile(path: string, callback: AsyncCallback<string[]>): void {
  let promise = taskpool.execute((path: string): string[] => {
    return FileIoImpl.listFileSync(path);
  }, path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string[];
    callback(e, r);
  }).catch((e: Error): void => {
    callback(e as BusinessError, []);
  });
}

function listFile(path: string, options: ListFileOptions, callback: AsyncCallback<string[]>): void {
  let promise = taskpool.execute((path: string, options: ListFileOptions): string[] => {
    return FileIoImpl.listFileSync(path, options);
  }, path, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string[];
    callback(e, r);
  }).catch((e: Error): void => {
    callback(e as BusinessError, []);
  });
}

function listFileSync(path: string, options?: ListFileOptions): string[] {
  return FileIoImpl.listFileSync(path, options);
}

function copyFileSync(src: string | int, dest: string | int, mode?: int): void {
  return FileIoImpl.copyFileSync(src, dest, mode)
}

function statSync(file: string | int): Stat {
  return FileIoImpl.statSync(file)
}

function stat(file: string | int): Promise<Stat> {
  return new Promise<Stat>((resolve: (result: Stat) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((file: string | int): Stat => {
      return FileIoImpl.statSync(file);
    }, file);
    promise.then((ret: NullishType): void => {
      let r = ret as Stat;
      resolve(r);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function stat(file: string | int, callback: AsyncCallback<Stat, void>): void {
  let p = taskpool.execute((file: string | int): Stat => {
    return FileIoImpl.statSync(file);
  }, file);
  p.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as Stat;
    callback(e, r);
  }).catch((e: Error): void => {
    callback(e as BusinessError, new StatInner(0));
  });
}

function fsyncSync(fd: int): void {
  return FileIoImpl.fsyncSync(fd);
}

function fsync(fd: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((fd: int): undefined => {
      return FileIoImpl.fsyncSync(fd);
    }, fd);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function fsync(fd: int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((fd: int): undefined => {
    return FileIoImpl.fsyncSync(fd);
  }, fd);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function symlinkSync(target: string, srcPath: string): void {
  return FileIoImpl.symlinkSync(target, srcPath);
}

function symlink(target: string, srcPath: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((target: string, srcPath: string): undefined => {
      return FileIoImpl.symlinkSync(target, srcPath);
    }, target, srcPath);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}
function renameSync(oldPath: string, newPath: string): void {
  return FileIoImpl.renameSync(oldPath, newPath);
}

function rename(oldPath: string, newPath: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((oldPath: string, newPath: string): undefined => {
      return FileIoImpl.renameSync(oldPath, newPath);
    }, oldPath, newPath);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function rename(oldPath: string, newPath: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((oldPath: string, newPath: string): undefined => {
    return FileIoImpl.renameSync(oldPath, newPath);
  }, oldPath, newPath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function createRandomAccessFileSync(file: string | File, mode?: int,
  options?: RandomAccessFileOptions): RandomAccessFile {
  return FileIoImpl.createRandomAccessFileSync(file, mode, options);
}

function createRandomAccessFile(file: string | File, mode?: int,
  options?: RandomAccessFileOptions): Promise<RandomAccessFile> {
  return new Promise<RandomAccessFile>((resolve: (result: RandomAccessFile) => void,
    reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((file: string | File, mode?: int,
      options?: RandomAccessFileOptions): RandomAccessFile => {
      return FileIoImpl.createRandomAccessFileSync(file, mode, options);
    }, file, mode, options);
    promise.then((ret: NullishType): void => {
      let raffile = ret as RandomAccessFileInner;
      resolve(raffile);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function fdopenStream(fd: int, mode: string): Promise<Stream> {
  return new Promise<Stream>((resolve: (result: Stream) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((fd: int, mode: string): Stream => {
      return FileIoImpl.fdopenStreamSync(fd, mode);
    }, fd, mode);
    promise.then((ret: NullishType): void => {
      let stream = ret as Stream;
      resolve(stream);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function setxattrSync(path: string, key: string, value: string): void {
  return FileIoImpl.setxattrSync(path, key, value)
}

function setxattr(path: string, key: string, value: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string, key: string, value: string): undefined =>
      FileIoImpl.setxattrSync(path, key, value), path, key, value);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function createRandomAccessFile(file: string | File, callback: AsyncCallback<RandomAccessFile, void>): void {
  let promise = taskpool.execute((file: string | File): RandomAccessFile => {
    return FileIoImpl.createRandomAccessFileSync(file);
  }, file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let raffile = ret as RandomAccessFile;
    callback(e, raffile);
  }).catch((e: Error): void => {
    let f: RandomAccessFile = new RandomAccessFileInner(0);
    callback(e as BusinessError, f);
  });
}

function createRandomAccessFile(file: string | File, mode: int,
  callback: AsyncCallback<RandomAccessFile, void>): void {
  let promise = taskpool.execute((file: string | File, mode: int): RandomAccessFile => {
    return FileIoImpl.createRandomAccessFileSync(file, mode);
  }, file, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let raffile = ret as RandomAccessFile;
    callback(e, raffile);
  }).catch((e: Error): void => {
    let f: RandomAccessFile = new RandomAccessFileInner(0);
    callback(e as BusinessError, f);
  });
}

function fdopenStream(fd: int, mode: string, callback: AsyncCallback<Stream>): void {
  let promise = taskpool.execute((fd: int, mode: string): Stream => {
    return FileIoImpl.fdopenStreamSync(fd, mode);
  }, fd, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let stream = ret as Stream;
    callback(e, stream);
  }).catch((e: Error): void => {
    let r: Stream = new StreamInner(0);
    callback(e as BusinessError, r);
  });
}

function fdopenStreamSync(fd: int, mode: string): Stream {
  return FileIoImpl.fdopenStreamSync(fd, mode);
}

function createStream(path: string, mode: string): Promise<Stream> {
  return new Promise<Stream>((resolve: (result: Stream) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: string, mode: string): Stream => {
      return FileIoImpl.createStreamSync(path, mode);
    }, path, mode);
    promise.then((ret: NullishType): void => {
      let stream = ret as Stream;
      resolve(stream);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function createStream(path: string, mode: string, callback: AsyncCallback<Stream>): void {
  let promise = taskpool.execute((path: string, mode: string): Stream => {
    return FileIoImpl.createStreamSync(path, mode);
  }, path, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let stream = ret as Stream;
    callback(e, stream);
  }).catch((e: Error): void => {
    let r: Stream = new StreamInner(0);
    callback(e as BusinessError, r);
  });
}

function createStreamSync(path: string, mode: string): Stream {
  return FileIoImpl.createStreamSync(path, mode);
}

function createReadStream(path: string, options?: ReadStreamOptions): ReadStream {
  return new ReadStream(path, options)
}

function createWriteStream(path: string, options?: WriteStreamOptions): WriteStream {
  return new WriteStream(path, options);
}

function createWatcher(path: string, events: int, listener: WatchEventListener): Watcher {
  return FileIoImpl.createWatcherSync(path, events, listener);
}

function symlink(target: string, srcPath: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((target: string, srcPath: string): undefined => {
    return FileIoImpl.symlinkSync(target, srcPath);
  }, target, srcPath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function utimes(path: string, mtime: double): void {
  return FileIoImpl.utimes(path, mtime);
}

function lstatSync(path: string): Stat {
  return FileIoImpl.lstatSync(path)
}

function lstat(path: string): Promise<Stat> {
  return new Promise<Stat>((resolve: (result: Stat) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute((path: string): Stat => {
      return FileIoImpl.lstatSync(path);
    }, path);
    promise.then((ret: NullishType): void => {
      if (ret === null || ret === undefined) {
        let e = new BusinessError<void>();
        e.code = -1;
        reject(e);
      } else {
        let r = ret as Stat;
        resolve(r);
      }
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function lstat(path: string, callback: AsyncCallback<Stat, void>): void {
  let p = taskpool.execute((path: string): Stat => {
    return FileIoImpl.lstatSync(path);
  }, path);
  p.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    if (ret === null || ret === undefined) {
      e.code = -1;
      let stat: Stat = new StatInner(0);
      callback(e, stat);
    } else {
      e.code = 0;
      let r = ret as Stat;
      callback(e, r);
    }
  }).catch((e: Error): void => {
    callback(e as BusinessError, new StatInner(0));
  });
}
function copyFile(src: string | int, dest: string | int, mode?: int): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((src: string | int, dest: string | int, mode?: int): undefined =>
      FileIoImpl.copyFileSync(src, dest, mode), src, dest, mode);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function copy(srcUri: string, destUri: string, options?: CopyOptions): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((srcUri: string, destUri: string, options?: CopyOptions): undefined =>
      FileIoImpl.copySync(srcUri, destUri, options), srcUri, destUri, options);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: Error): void => {
      reject(e as BusinessError);
    });
  });
}

function copyFile(src: string | int, dest: string | int, mode: int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string | int, dest: string | int, mode: int): undefined =>
    FileIoImpl.copyFileSync(src, dest, mode), src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function copy(srcUri: string, destUri: string, options: CopyOptions, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((srcUri: string, destUri: string, options: CopyOptions): undefined =>
    FileIoImpl.copySync(srcUri, destUri, options), srcUri, destUri, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function copyFile(src: string | int, dest: string | int, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string | int, dest: string | int): undefined =>
    FileIoImpl.copyFileSync(src, dest), src, dest);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function copy(srcUri: string, destUri: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((srcUri: string, destUri: string): undefined =>
    FileIoImpl.copySync(srcUri, destUri), srcUri, destUri);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: Error): void => {
    callback(e as BusinessError, undefined);
  });
}

function lseek(fd: int, offset: long, whence?: WhenceType): long {
  return FileIoImpl.lseekSync(fd, offset, whence);
}

export interface Progress {
  processedSize: long;
  totalSize: long;
}

export interface DfsListeners {
  onStatus(networkId: string, status: number): void;
}

export class ProgressInner implements Progress {
  processedSize: long;
  totalSize: long;

  constructor(pSize: long, tSize: long) {
    this.processedSize = pSize;
    this.totalSize = tSize;
  }
}

export type ProgressListener = (progress: Progress) => void;

export class TaskSignal {
  static {
    loadLibrary("ani_file_fs");
  }
  private nativeTaskSignal: long = 0;
  native constructor();
  private native onCancelNative(): void;

  private onCancelResolve: (path: string) => void = (path: string): void => {};
  private onCancelCallback(path: string): void {
    if (this.onCancelResolve) {
      this.onCancelResolve(path);
    }
  }

  native cancel(): void;

  onCancel(): Promise<string> {
    return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
      this.onCancelResolve = resolve;
      try {
        this.onCancelNative();
      } catch (e: BusinessError<void>) {
        reject(e);
      } catch (e: Error) {
        reject(e as BusinessError);
      }
    });
  }

}

export interface CopyOptions {
  progressListener?: ProgressListener;
  copySignal?: TaskSignal;
}

export enum AccessModeType {
  EXIST = 0,
  WRITE = 2,
  READ = 4,
  READ_WRITE = 6,
}

export enum AccessFlagType {
  LOCAL = 0,
}

export interface RandomAccessFile {
  fd: int;
  filePointer: long;

  setFilePointer(filePointer: long): void;
  close(): void;
  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<long>;
  write(buffer: ArrayBuffer | string, callback: AsyncCallback<long>): void;
  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<long>): void;
  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): long;
  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<long>;
  read(buffer: ArrayBuffer, callback: AsyncCallback<long>): void;
  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long>): void;
  readSync(buffer: ArrayBuffer, options?: ReadOptions): long;
  getReadStream(): ReadStream;
  getWriteStream(): WriteStream;
}

export class RandomAccessFileInner implements RandomAccessFile {
  fd: int = -1;
  filePointer: long = -1;

  private nativePtr: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
      this.registerCleaner(this.nativePtr);
    }
  }

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "RandomAccessFile");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  setFilePointer(filePointer: long): void {
    this.setFilePointer0(filePointer);
    this.filePointer = filePointer;
  }

  native setFilePointer0(filePointer: long): void;

  native close(): void;

  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): long {
    let length = options ? this.writeSync0(buffer, options) : this.writeSync0(buffer);
    this.filePointer += (options?.offset?? 0) + length;
    return length;
  }

  native writeSync0(buffer: ArrayBuffer | string, options?: WriteOptions): long;

  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute((buffer: ArrayBuffer | string, options?: WriteOptions): long => {
        return this.writeSync(buffer, options);
      }, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as long
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<long, void>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer | string, options: WriteOptions): long => {
      return this.writeSync(buffer, options);
    }, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  write(buffer: ArrayBuffer | string, callback: AsyncCallback<long, void>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer | string): long => {
      return this.writeSync(buffer);
    }, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  readSync(buffer: ArrayBuffer, options?: ReadOptions): long {
    const length = options ? this.readSync0(buffer, options) : this.readSync0(buffer);
    this.filePointer += (options?.offset?? 0) + length;
    return length;
  }

  native readSync0(buffer: ArrayBuffer, options?: ReadOptions): long;

  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute((buffer: ArrayBuffer, options?: ReadOptions): long => {
        return this.readSync(buffer, options);
      }, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as long;
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long, void>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer, options: ReadOptions): long => {
      return this.readSync(buffer, options);
    }, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  read(buffer: ArrayBuffer, callback: AsyncCallback<long, void>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer): long => {
      return this.readSync(buffer);
    }, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  native getReadStream(): ReadStream;
  native getWriteStream(): WriteStream;
}

export interface File {
  fd: int;
  path: String;
  name: String;

  getParent(): String;
  lock(exclusive?: boolean): Promise<void>;
  lock(callback: AsyncCallback<void>): void;
  lock(exclusive: boolean, callback: AsyncCallback<void>): void;
  tryLock(exclusive?: boolean): void;
  unlock(): void;
}

export class FileInner implements File {
  fd: int = -1;
  path: String = "";
  name: String = "";

  private nativePtr: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
      this.registerCleaner(this.nativePtr);
    }
  }

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "File");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  native getParent(): String;
  native lockSync(exclusive?: boolean): void;

  lock(exclusive?: boolean): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute((exclusive?: boolean): undefined => {
        return this.lockSync(exclusive);
      }, exclusive);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  lock(callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((): undefined => {
      return this.lockSync();
    });
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: Error): void => {
      callback(e as BusinessError, undefined);
    });
  }

  lock(exclusive: boolean, callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((exclusive: boolean): undefined => {
      return this.lockSync(exclusive);
    }, exclusive);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: Error): void => {
      callback(e as BusinessError, undefined);
    });
  }

  native tryLock(exclusive?: boolean): void;
  native unlock(): void;

}

export enum LocationType {
  LOCAL = 1,
  CLOUD = 2
}

export class ReaderIteratorResultInner implements ReaderIteratorResult {
  done: boolean = false;
  value: string = "";
  
  constructor(done: boolean, value: string) {
    this.done = done;
    this.value = value;
  }
}

export interface ReaderIterator {
  next(): ReaderIteratorResult;
}

export class ReaderIteratorInner implements ReaderIterator {
  private nativePtr: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
      this.registerCleaner(this.nativePtr);
    }
  }

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "ReaderIterator");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  native next(): ReaderIteratorResult;
}

export interface Stat {
  ino: bigint;
  mode: long;
  uid: long;
  gid: long;
  size: long;
  atime: long;
  mtime: long;
  ctime: long;
  atimeNs: bigint;
  mtimeNs: bigint;
  ctimeNs: bigint;
  location: LocationType;

  isBlockDevice(): boolean;
  isCharacterDevice(): boolean;
  isDirectory(): boolean;
  isFIFO(): boolean;
  isFile(): boolean;
  isSocket(): boolean;
  isSymbolicLink(): boolean;
}

export class StatInner implements Stat {
  ino: bigint = 0n;
  mode: long;
  uid: long;
  gid: long;
  size: long;
  atime: long;
  mtime: long;
  ctime: long;
  atimeNs: bigint = 0n;
  mtimeNs: bigint = 0n;
  ctimeNs: bigint = 0n;
  location: LocationType = LocationType.LOCAL;

  private nativeStat: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(stat: long) {
    if (this.nativeStat === 0) {
      this.nativeStat = stat;
      this.registerCleaner(this.nativeStat)
    }
  }

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "Stat");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  native isBlockDevice(): boolean;
  native isCharacterDevice(): boolean;
  native isDirectory(): boolean;
  native isFIFO(): boolean;
  native isFile(): boolean;
  native isSocket(): boolean;
  native isSymbolicLink(): boolean;
}

export interface Stream {
  close(): Promise<void>;
  close(callback: AsyncCallback<void>): void;
  closeSync(): void;
  flush(): Promise<void>;
  flush(callback: AsyncCallback<void>): void;
  flushSync(): void;
  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<long>;
  write(buffer: ArrayBuffer | string, callback: AsyncCallback<long>): void;
  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<long>): void;
  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): long;
  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<long>;
  read(buffer: ArrayBuffer, callback: AsyncCallback<long>): void;
  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long>): void;
  readSync(buffer: ArrayBuffer, options?: ReadOptions): long;
  seek(offset: long, whence?: int): long;
}

export class StreamInner implements Stream {
  private nativePtr: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
      this.registerCleaner(this.nativePtr)
    }
  }

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "Stream");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  close(): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute((): undefined => this.closeSync());
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  close(callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((): undefined => this.closeSync());
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: Error): void => {
      callback(e as BusinessError, undefined);
    });
  }

  native closeSync(): void;

  flush(): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute((): undefined => this.flushSync());
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  flush(callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((): undefined => this.flushSync());
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: Error): void => {
      callback(e as BusinessError, undefined);
    });
  }

  native flushSync(): void;

  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute((buffer: ArrayBuffer | string, options?: WriteOptions): long => {
        return this.writeSync(buffer, options);
      }, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as long
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  write(buffer: ArrayBuffer | string, callback: AsyncCallback<long>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer | string): long => {
      return this.writeSync(buffer);
    }, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<long>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer | string, options: WriteOptions): long => {
      return this.writeSync(buffer, options);
    }, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  native writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): long;

  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute((buffer: ArrayBuffer, options?: ReadOptions): long => {
        return this.readSync(buffer, options);
      }, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as long
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  read(buffer: ArrayBuffer, callback: AsyncCallback<long>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer): long => {
      return this.readSync(buffer);
    }, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long>): void {
    let promise = taskpool.execute((buffer: ArrayBuffer, options: ReadOptions): long => {
      return this.readSync(buffer, options);
    }, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as long;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  native readSync(buffer: ArrayBuffer, options?: ReadOptions): long;
  native seek(offset: long, whence?: int): long;
}

export class ReadStream extends stream.Readable {
  path: string;
  bytesRead: long;
  private offset: long;
  private start?: long;
  private end?: long;
  private stream?: Stream;

  constructor(path: string, options?: ReadStreamOptions) {
    super();
    this.path = path;
    this.bytesRead = 0;
    this.start = options?.start;
    this.end = options?.end;
    this.stream = createStreamSync(this.path, 'r');
    this.offset = this.start ?? 0;
  }

  seek(offset: long, whence?: WhenceType): long {
    if (whence === undefined) {
      let off = this.stream?.seek(offset);
      if (off !== undefined) {
        this.offset = off
      }
    } else {
      let off = this.stream?.seek(offset, whence);
      if (off !== undefined) {
        this.offset = off
      }
    }
    return this.offset;
  }

  close(): void {
    this.stream?.close();
  }

  doInitialize(callback: Function): void {
    callback.unsafeCall();
  }

  doRead(size: int): void {
    let readSize = size;
    let end = this.end
    if (end !== undefined) {
      if (this.offset > end) {
        this.push(null);
        return;
      }
      if (this.offset + readSize > end) {
        readSize = (end - this.offset).toInt();
      }
    }
    let buffer = new ArrayBuffer(readSize);
    const off = this.offset;
    this.offset += readSize;
    this.stream?.read(buffer, { offset: off, length: readSize })
      .then((readOut: long) => {
        if (readOut > 0) {
          this.bytesRead += readOut;
          this.push(new Uint8Array(buffer.slice(0, readOut)));
        }
        if (readOut !== readSize || readOut < size) {
          this.offset = this.offset - readSize + readOut;
          this.push(null);
        }
      });
  }
}

export class WriteStream extends stream.Writable {
  path: string;
  bytesWritten: long;
  private offset: long;
  private mode: string;
  private start?: long;
  private stream?: Stream;

  constructor(path: string, options?: WriteStreamOptions) {
    super();
    this.path = path;
    this.bytesWritten = 0;
    this.start = options?.start;
    this.mode = this.convertOpenMode(options?.mode);
    this.stream = createStreamSync(this.path, this.mode);
    this.offset = this.start ?? 0;
  }

  seek(offset: long, whence?: WhenceType): long {
    if (whence === undefined) {
      let off = this.stream?.seek(offset);
      if (off !== undefined) {
        this.offset = off
      }
    } else {
      let off = this.stream?.seek(offset, whence);
      if (off !== undefined) {
        this.offset = off
      }
    }
    return this.offset;
  }

  close(): void {
    this.stream?.close();
  }

  closeSync(): void {
    this.stream?.closeSync();
  }

  doInitialize(callback: Function): void {
    callback.unsafeCall();
  }

  doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
    let buf: string | ArrayBuffer;
    if (chunk instanceof Uint8Array) {
      buf = chunk.buffer;
    } else {
      buf = chunk;
    }
    this.stream?.write(buf, { offset: this.offset })
      .then((writeIn: long) => {
        this.offset += writeIn;
        this.bytesWritten += writeIn;
        callback.unsafeCall();
      })
      .finally(() => {
        this.stream?.flush();
      });
  }

  convertOpenMode(mode?: int): string {
    let modeStr = 'w';
    if (mode === undefined) {
      return modeStr;
    }
    if ((mode as int) & fileIo.OpenMode.WRITE_ONLY) {
      modeStr = 'w';
    }
    if ((mode as int) & fileIo.OpenMode.READ_WRITE) {
      modeStr = 'w+';
    }
    if (((mode as int) & fileIo.OpenMode.WRITE_ONLY) && ((mode as int) & fileIo.OpenMode.APPEND)) {
      modeStr = 'a';
    }
    if (((mode as int) & fileIo.OpenMode.READ_WRITE) && ((mode as int) & fileIo.OpenMode.APPEND)) {
      modeStr = 'a+';
    }
    return modeStr;
  }
}

export class AtomicFile {
  static {
    loadLibraryWithPermissionCheck("ani_file_fs", "@ohos.file.fs");
  }

  private nativePtr: long = 0;
  private cleaner: Cleaner | null = null;
  private native getPath(): string;
  private writeStream: WriteStream | null = null;

  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr, "AtomicFile");
      destroyRegister.register(this, this.cleaner!, this);
  }

  unregisterCleaner(): void {
      destroyRegister.unregister(this);
  }

  bindNativePtr(ptr: long): void {
      if (this.nativePtr === 0) {
          this.nativePtr = ptr;
          this.registerCleaner(this.nativePtr)
      }
  }

  native constructor(path: string);

  native getBaseFile(): File;

  native openRead(): ReadStream;

  native readFully(): ArrayBuffer;

  native nativeStartWrite(): WriteStream;
  startWrite(): WriteStream {
    let ws = this.nativeStartWrite();
    this.writeStream = ws;
    return ws;
  }

  native nativeFinishWrite(): void;
  finishWrite(): void {
    if (!this.writeStream) {
      throw createBusinessError(UNKNOWN_ERR, UNKNOWN_MSG);
    }
    this.writeStream?.close();
    this.nativeFinishWrite();
    this.writeStream = null;
  };

  native nativeFailWrite(): void;
  failWrite(): void {
    if (!this.writeStream) {
      throw createBusinessError(UNKNOWN_ERR, UNKNOWN_MSG);
    }
    this.writeStream?.close();
    this.nativeFailWrite();
    this.writeStream = null;
  };

  native delete(): void;
}

export enum WhenceType {
  SEEK_SET = 0,
  SEEK_CUR = 1,
  SEEK_END = 2
}

export interface Watcher {
  start(): void;
  stop(): void;
}

class WatcherInner implements Watcher {
  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  native start(): void;

  native stop(): void;
}

} // namespace fileIo

export interface ConflictFiles {
  srcFile: string;
  destFile: string;
}

class ConflictFilesInner implements ConflictFiles {
  srcFile: string = "";
  destFile: string = "";

  constructor(src: string, dest: string) {
    this.srcFile = src;
    this.destFile = dest;
  }
}

export interface WatchEvent {
  fileName: string;
  event: int;
  cookie: int;
}

class WatchEventInner implements WatchEvent {
  fileName: string = '';
  event: int;
  cookie: int;

  constructor(fileName: string, event: int, cookie: int) {
    this.fileName = fileName;
    this.event = event;
    this.cookie = cookie;
  }

}

export type WatchEventListener = (event: WatchEvent) => void;

export interface Options {
  encoding?: string;
}

export interface ReadOptions {
  offset?: long;
  length?: long;
}

export interface ReadTextOptions extends ReadOptions {
  encoding?: string;
}

export interface WriteOptions extends Options {
  offset?: long;
  length?: long;
}

export interface RandomAccessFileOptions {
  start?: long;
  end?: long;
}

export interface ListFileOptions {
  recursion?: boolean;
  listNum?: long;
  filter?: Filter;
}

export interface WriteStreamOptions {
  mode?: int;
  start?: long;
}

export interface ReadStreamOptions {
  start?: long;
  end?: long;
}

export class ReadStreamOptionsInner implements ReadStreamOptions {
  constructor() {}
  start?: long;
  end?: long;
}

export class WriteStreamOptionsInner implements WriteStreamOptions {
  constructor() {}
  mode?: int;
  start?: long;
}

export interface ReaderIteratorResult {
  done: boolean;
  value: string;
}

export interface Filter {
  suffix?: Array<string>;
  displayName?: Array<string>;
  mimeType?: Array<string>;
  fileSizeOver?: long;
  lastModifiedAfter?: double;
  excludeMedia?: boolean;
}

type TaskSignal = fileIo.TaskSignal;
type DfsListeners = fileIo.DfsListeners;


export class FileIoImpl {

  static {
    loadLibraryWithPermissionCheck("ani_file_fs", "@ohos.file.fs");
  }

  static native doAccessSync(path: string, mode?: fileIo.AccessModeType, flag?: fileIo.AccessFlagType): boolean;

  static native closeSync(file: int | fileIo.File): void;

  static native copySync(srcUri: string, destUri: string, options?: fileIo.CopyOptions): void;

  static native connectDfs(networkId: string, listeners: DfsListeners): void;

  static native copyDirSync(src: string, dest: string, mode?: int): void;

  static native copyFileSync(src: string | int, dest: string | int, mode?: int): void;

  static native disConnectDfs(networkId: string): void;

  static native fdatasyncSync(fd: int): void;

  static native getxattrSync(path: string, key: string): string;

  static native createStreamSync(path: string, mode: string): fileIo.Stream;

  static native createWatcherSync(path: string, events: int, listener: WatchEventListener): fileIo.Watcher;

  static native fdopenStreamSync(fd: int, mode: string): fileIo.Stream;

  static native dup(fd: int): fileIo.File;

  static native listFileSync(path: string, options?: ListFileOptions): string[];

  static native lstatSync(path: string): fileIo.Stat;

  static native lseekSync(fd: int, offset: long, whence?: fileIo.WhenceType): long;

  static native mkdirSync(path: string): void;

  static native mkdirSync(path: string, recursion: boolean): void;

  static native movedirSync(src: string, dest: string, mode?: int): void;

  static native mkdtempSync(prefix: string): string;

  static native moveFileSync(src: String, dest: String, mode?: int): void;

  static native openSync(path: String, mode?: int): fileIo.File;

  static native readlinesSync(filePath: string, options?: Options): fileIo.ReaderIterator;

  static native readSync(fd: int, buffer: ArrayBuffer, options?: ReadOptions): long;

  static native readTextSync(filePath: string, options?: ReadTextOptions): string;

  static native rmdirSync(path: string): void;

  static native setxattrSync(path: string, key: string, value: string): void;

  static native statSync(file: string | int): fileIo.Stat;

  static native truncateSync(file: string | int, len?: long): void;

  static native unlinkSync(path: string): void;

  static native writeSync(fd: int, buffer: string | ArrayBuffer, options?: WriteOptions): long;

  static native fsyncSync(fd: int): void;

  static native renameSync(oldPath: string, newPath: string): void;

  static native createRandomAccessFileSync(file: string | fileIo.File, mode?: int,
    options?: RandomAccessFileOptions): fileIo.RandomAccessFile;

  static native symlinkSync(target: string, srcPath: string): void;

  static native utimes(path: string, mtime: double): void;
}
