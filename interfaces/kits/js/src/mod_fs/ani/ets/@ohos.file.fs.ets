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

const UNKNOWN_ERR: number = 13900042
const UNKNOWN_MSG: string = "Unknown error"

function createBusinessError(code: number, msg: string): BusinessError {
  let err = new BusinessError();
  err.code = code;
  err.message = msg;
  return err;
}

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
    if (mode === undefined) {
      let promise = taskpool.execute((path: string): boolean => {
        return FileIoImpl.doAccessSync(path);
      }, path);
      promise.then((ret: NullishType) => {
        let result = ret as boolean;
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((path: string, mode: AccessModeType): boolean => {
        return FileIoImpl.doAccessSync(path, mode);
      }, path, mode);
      promise.then((ret: NullishType) => {
        let result = ret as boolean;
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
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
  }).catch((e: BusinessError): void => {
    callback(e, false);
  });
}

function access(path: string, mode: AccessModeType, flag: AccessFlagType): Promise<boolean> {
  return new Promise<boolean>((resolve: (result: boolean) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.doAccessSync, path, mode, flag);
    promise.then((ret: NullishType): void => {
      let result = ret as boolean;
      resolve(result);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  })
}

function accessSync(path: string, mode?: AccessModeType): boolean {
  return FileIoImpl.doAccessSync(path, mode);
}

function accessSync(path: string, mode: AccessModeType, flag: AccessFlagType): boolean {
  return FileIoImpl.doAccessSync(path, mode, flag);
}

function close(file: number | File): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((file: number | File): undefined => FileIoImpl.closeSync(file), file);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function close(file: number | File, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: number | File): undefined => FileIoImpl.closeSync(file), file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function closeSync(file: number | File): void {
  return FileIoImpl.closeSync(file)
}

function connectDfs(networkId: string, listeners: DfsListeners): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((networkId: string, listeners: DfsListeners): void =>
      FileIoImpl.connectDfs(networkId, listeners), networkId, listeners);
    promise.then<void>((): void => {
      resolve(undefined);
    }).catch((e: BusinessError<void>): void => {
      reject(e);
    });
  });
}

function disconnectDfs(networkId: string): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((networkId: string): void => FileIoImpl.disConnectDfs(networkId), networkId);
    promise.then<void>((): void => {
      resolve(undefined);
    }).catch((e: BusinessError<void>): void => {
      reject(e);
    });
  });
}

function getxattrSync(path: string, key: string): string {
  return FileIoImpl.getxattrSync(path, key)
}

function getxattr(path: string, key: string): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute(FileIoImpl.getxattrSync, path, key);
    promise.then((ret: NullishType): void => {
      let result = ret as string;
      resolve(result);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function dup(fd: number): File {
  return FileIoImpl.dup(fd);
}

function copyDirSync(src: string, dest: string, mode?: number): void {
  return FileIoImpl.copyDirSync(src, dest, mode);
}

function copyDir(src: string, dest: string, mode?: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<Array<ConflictFiles>>) => void): void => {
    if (mode === undefined) {
      let promise = taskpool.execute((src: string, dest: string): undefined =>
        FileIoImpl.copyDirSync(src, dest), src, dest);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((src: string, dest: string, mode: number): undefined =>
        FileIoImpl.copyDirSync(src, dest, mode), src, dest, mode);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
        reject(e);
      });
    }

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
  }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
    callback(e, undefined);
  });
}

function copyDir(src: string, dest: string, mode: number, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: number): undefined =>
    FileIoImpl.copyDirSync(src, dest, mode), src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
    callback(e, undefined);
  });
}

function copyDir(arg0: Object, arg1: Object, arg2: NullishType, arg3: NullishType): void {
  let src = arg0 as string;
  let dest = arg1 as string;
  if (typeof arg2 === 'number' && typeof arg3 === 'function') {
    let mode: number = 0;
    try {
      mode = arg2 as number;
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

function fdatasyncSync(fd: number): void {
  return FileIoImpl.fdatasyncSync(fd)
}

function fdatasync(fd: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((fd: number): undefined => FileIoImpl.fdatasyncSync(fd), fd);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: BusinessError): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function fdatasync(fd: number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((fd: number): undefined => FileIoImpl.fdatasyncSync(fd), fd);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
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
    }).catch((e: BusinessError): void => {
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
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
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
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function moveDirSync(src: string, dest: string, mode?: number): void {
  return FileIoImpl.movedirSync(src, dest, mode)
}

function moveDir(src: string, dest: string, mode?: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void,
    reject: (e: BusinessError<Array<ConflictFiles>>) => void): void => {
    if (mode === undefined) {
      let promise = taskpool.execute((src: string, dest: string): undefined => {
        return FileIoImpl.movedirSync(src, dest);
      }, src, dest);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((src: string, dest: string, mode: number): undefined => {
        return FileIoImpl.movedirSync(src, dest, mode);
      }, src, dest, mode);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
        reject(e);
      });
    }
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
  }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
    callback(e, undefined);
  });
}

function moveDir(src: string, dest: string, mode: number, callback: AsyncCallback<void, Array<ConflictFiles>>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: number): undefined => {
    return FileIoImpl.movedirSync(src, dest, mode);
  }, src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<Array<ConflictFiles>>();
    e.code = 0;
    e.data = new Array<ConflictFiles>(0);
    callback(e, undefined);
  }).catch((e: BusinessError<Array<ConflictFiles>>): void => {
    callback(e, undefined);
  });
}

function moveDir(arg0: Object, arg1: Object, arg2: NullishType, arg3: NullishType): void {
  let src = arg0 as string;
  let dest = arg1 as string;
  if (typeof arg2 === 'number' && typeof arg3 === 'function') {
    let mode: number = 0;
    try {
      mode = arg2 as number;
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
    let promise = taskpool.execute(FileIoImpl.mkdtempSync, prefix);
    promise.then((ret: NullishType): void => {
      let result = ret as string;
      resolve(result);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function mkdtemp(prefix: string, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute(FileIoImpl.mkdtempSync, prefix);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as string;
    callback(e, result);
  }).catch((e: BusinessError): void => {
    callback(e, "");
  });
}

function moveFileSync(src: string, dest: string, mode?: number): void {
  return FileIoImpl.moveFileSync(src, dest, mode);
}

function moveFile(src: string, dest: string, mode?: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    if (mode === undefined) {
      let promise = taskpool.execute((src: string, dest: string): undefined => {
        return FileIoImpl.moveFileSync(src, dest);
      }, src, dest);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((src: string, dest: string, mode: number): undefined => {
        return FileIoImpl.moveFileSync(src, dest, mode);
      }, src, dest, mode);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function moveFile(src: string, dest: string, mode: number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string, dest: string, mode: number): undefined => {
    return FileIoImpl.moveFileSync(src, dest, mode);
  }, src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
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
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function openSync(path: string, mode?: number): File {
  return FileIoImpl.openSync(path, mode);
}

function open(path: String, mode?: number): Promise<File> {
  return new Promise<File>((resolve: (result: File) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.openSync, path, mode);
    promise.then((ret: NullishType): void => {
      let file = ret as File;
      resolve(file);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function open(path: String, mode: number, callback: AsyncCallback<File, void>): void {
  let promise = taskpool.execute(FileIoImpl.openSync, path, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let file = ret as File;
    callback(e, file);
  }).catch((e: BusinessError): void => {
    let f: File = new FileInner(0);
    callback(e, f);
  });
}

function open(path: String, callback: AsyncCallback<File, void>): void {
  let promise = taskpool.execute(FileIoImpl.openSync, path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let file = ret as File;
    callback(e, file);
  }).catch((e: BusinessError): void => {
    let f: File = new FileInner(0);
    callback(e, f);
  });
}

function writeSync(fd: number, buffer: string | ArrayBuffer, options?: WriteOptions): number {
  return FileIoImpl.writeSync(fd, buffer, options);
}

function write(fd: number, buffer: string | ArrayBuffer, options?: WriteOptions): Promise<number> {
  return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
    if (options === undefined) {
      let promise = taskpool.execute(FileIoImpl.writeSync, fd, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as number
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute(FileIoImpl.writeSync, fd, buffer, options);
      promise.then((ret: NullishType): void => {
        let result = ret as number
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function write(fd: number, buffer: string | ArrayBuffer, options: WriteOptions,
  callback: AsyncCallback<number, void>): void {
  let promise = taskpool.execute(FileIoImpl.writeSync, fd, buffer, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as number;
    callback(e, result);
  }).catch((e: BusinessError): void => {
    callback(e, 0);
  });
}

function write(fd: number, buffer: string | ArrayBuffer, callback: AsyncCallback<number, void>): void {
  let promise = taskpool.execute(FileIoImpl.writeSync, fd, buffer);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as number;
    callback(e, result);
  }).catch((e: BusinessError): void => {
    callback(e, 0);
  });
}

function readSync(fd: number, buffer: ArrayBuffer, options?: ReadOptions): number {
  return FileIoImpl.readSync(fd, buffer, options)
}

function read(fd: number, buffer: ArrayBuffer, options?: ReadOptions): Promise<number> {
  return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
    if (options === undefined) {
      let promise = taskpool.execute((fd: number, buffer: ArrayBuffer): number => {
        return FileIoImpl.readSync(fd, buffer);
      }, fd, buffer);
      promise.then((ret: NullishType) => {
        let result = ret as number;
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((fd: number, buffer: ArrayBuffer, options: ReadOptions): number => {
        return FileIoImpl.readSync(fd, buffer, options)
      }, fd, buffer, options);
      promise.then((ret: NullishType) => {
        let result = ret as number;
        resolve(result);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function read(fd: number, buffer: ArrayBuffer, callback: AsyncCallback<number, void>): void {
  let promise = taskpool.execute(FileIoImpl.readSync, fd, buffer);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as number;
    callback(e, result);
  }).catch((e: BusinessError): void => {
    callback(e, 0);
  });
}

function read(fd: number, buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<number, void>): void {
  let promise = taskpool.execute(FileIoImpl.readSync, fd, buffer, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let result = ret as number;
    callback(e, result);
  }).catch((e: BusinessError): void => {
    callback(e, 0);
  });
}

function readLinesSync(filePath: string, options?: Options): ReaderIterator {
  return FileIoImpl.readlinesSync(filePath, options)
}

function readLines(filePath: string, options?: Options): Promise<ReaderIterator> {
  return new Promise<ReaderIterator>((resolve: (result: ReaderIterator) => void, reject: (e: BusinessError<void>) => void) => {
    if (options === undefined) {
      let promise = taskpool.execute(FileIoImpl.readlinesSync, filePath);
      promise.then((ret: NullishType): void => {
        let it = ret as ReaderIterator;
        resolve(it);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute(FileIoImpl.readlinesSync, filePath, options);
      promise.then((ret: NullishType): void => {
        let it = ret as ReaderIterator;
        resolve(it);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function readLines(filePath: string, callback: AsyncCallback<ReaderIterator>): void {
  let promise = taskpool.execute(FileIoImpl.readlinesSync, filePath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let it = ret as ReaderIterator;
    callback(e, it);
  }).catch((e: BusinessError): void => {
    let r: ReaderIterator = new ReaderIteratorInner(0);
    callback(e, r);
  });
}

function readLines(filePath: string, options: Options, callback: AsyncCallback<ReaderIterator>): void {
  let promise = taskpool.execute(FileIoImpl.readlinesSync, filePath, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let it = ret as ReaderIterator;
    callback(e, it);
  }).catch((e: BusinessError): void => {
    let r: ReaderIterator = new ReaderIteratorInner(0);
    callback(e, r);
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
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function rmdir(path: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string): void => FileIoImpl.rmdirSync(path), path);
  promise.then((ret: NullishType) => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function truncateSync(file: string | number, len?: number): void {
  return FileIoImpl.truncateSync(file, len)
}

function truncate(file: string | number, len?: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    if (len === undefined) {
      let promise = taskpool.execute((file: string | number): undefined => {
        return FileIoImpl.truncateSync(file);
      }, file);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((file: string | number, len: number): undefined => {
        return FileIoImpl.truncateSync(file, len);
      }, file, len);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  })
}

function truncate(file: string | number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: string | number): undefined => {
    return FileIoImpl.truncateSync(file);
  }, file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function truncate(file: string | number, len: number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((file: string | number, len: number): undefined => {
    return FileIoImpl.truncateSync(file, len);
  }, file, len);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
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
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function unlink(path: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string): undefined => unlinkSync(path), path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function readText(filePath: string, options?: ReadTextOptions): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void) => {
    if (options === undefined) {
      let promise = taskpool.execute(FileIoImpl.readTextSync, filePath);
      promise.then((ret: NullishType): void => {
        let r = ret as string;
        resolve(r);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute(FileIoImpl.readTextSync, filePath, options);
      promise.then((ret: NullishType): void => {
        let r = ret as string;
        resolve(r);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function readText(filePath: string, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute(FileIoImpl.readTextSync, filePath);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string;
    callback(e, r);
  }).catch((e: BusinessError): void => {
    callback(e, "");
  });
}

function readText(filePath: string, options: ReadTextOptions, callback: AsyncCallback<string>): void {
  let promise = taskpool.execute(FileIoImpl.readTextSync, filePath, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string;
    callback(e, r);
  }).catch((e: BusinessError): void => {
    callback(e, "");
  });
}

function readTextSync(filePath: string, options?: ReadTextOptions): string {
  return FileIoImpl.readTextSync(filePath, options);
}

function listFile(path: string, options?: ListFileOptions): Promise<string[]> {
  return new Promise<string[]>((resolve: (result: string[]) => void, reject: (e: BusinessError<void>) => void) => {
    if (options === undefined) {
      let promise = taskpool.execute(FileIoImpl.listFileSync, path);
      promise.then((ret: NullishType): void => {
        let r = ret as string[];
        resolve(r);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute(FileIoImpl.listFileSync, path, options);
      promise.then((ret: NullishType): void => {
        let r = ret as string[];
        resolve(r);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function listFile(path: string, callback: AsyncCallback<string[]>): void {
  let promise = taskpool.execute(FileIoImpl.listFileSync, path);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string[];
    callback(e, r);
  }).catch((e: BusinessError): void => {
    callback(e, []);
  });
}

function listFile(path: string, options: ListFileOptions, callback: AsyncCallback<string[]>): void {
  let promise = taskpool.execute(FileIoImpl.listFileSync, path, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as string[];
    callback(e, r);
  }).catch((e: BusinessError): void => {
    callback(e, []);
  });
}

function listFileSync(path: string, options?: ListFileOptions): string[] {
  return FileIoImpl.listFileSync(path, options);
}

function copyFileSync(src: string | number, dest: string | number, mode?: number): void {
  return FileIoImpl.copyFileSync(src, dest, mode)
}

function statSync(file: string | number): Stat {
  return FileIoImpl.statSync(file)
}

function stat(file: string | number): Promise<Stat> {
  return new Promise<Stat>((resolve: (result: Stat) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.statSync, file);
    promise.then((ret: NullishType): void => {
      let r = ret as Stat;
      resolve(r);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function stat(file: string | number, callback: AsyncCallback<Stat, void>): void {
  let p = taskpool.execute(FileIoImpl.statSync, file);
  p.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let r = ret as Stat;
    callback(e, r);
  }).catch((e: BusinessError): void => {
    callback(e, new StatInner(0));
  });
}

function fsyncSync(fd: number): void {
  return FileIoImpl.fsyncSync(fd);
}

function fsync(fd: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((fd: number): undefined => {
      return FileIoImpl.fsyncSync(fd);
    }, fd);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function fsync(fd: number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((fd: number): undefined => {
    return FileIoImpl.fsyncSync(fd);
  }, fd);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
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
    }).catch((e: BusinessError): void => {
      reject(e);
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
    }).catch((e: BusinessError): void => {
      reject(e);
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
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function createRandomAccessFileSync(file: string | File, mode?: number,
  options?: RandomAccessFileOptions): RandomAccessFile {
  return FileIoImpl.createRandomAccessFileSync(file, mode, options);
}

function createRandomAccessFile(file: string | File, mode?: number,
  options?: RandomAccessFileOptions): Promise<RandomAccessFile> {
  return new Promise<RandomAccessFile>((resolve: (result: RandomAccessFile) => void,
    reject: (e: BusinessError<void>) => void) => {
    if (mode === undefined) {
      let promise = taskpool.execute(FileIoImpl.createRandomAccessFileSync, file);
      promise.then((ret: NullishType): void => {
        let raffile = ret as RandomAccessFileInner;
        resolve(raffile);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
      return;
    }
    if (mode !== undefined && options === undefined) {
      let promise = taskpool.execute(FileIoImpl.createRandomAccessFileSync, file, mode);
      promise.then((ret: NullishType): void => {
        let raffile = ret as RandomAccessFileInner;
        resolve(raffile);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
      return;
    }
    let promise = taskpool.execute(FileIoImpl.createRandomAccessFileSync, file, mode, options);
    promise.then((ret: NullishType): void => {
      let raffile = ret as RandomAccessFileInner;
      resolve(raffile);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}
function fdopenStream(fd: number, mode: string): Promise<Stream> {
  return new Promise<Stream>((resolve: (result: Stream) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.fdopenStreamSync, fd, mode);
    promise.then((ret: NullishType): void => {
      let stream = ret as Stream;
      resolve(stream);
    }).catch((e: BusinessError): void => {
      reject(e);
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
    }).catch((e: BusinessError): void => {
      reject(e as BusinessError<void>);
    });
  });
}

function createRandomAccessFile(file: string | File, callback: AsyncCallback<RandomAccessFile, void>): void {
  let promise = taskpool.execute(FileIoImpl.createRandomAccessFileSync, file);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let raffile = ret as RandomAccessFile;
    callback(e, raffile);
  }).catch((e: BusinessError): void => {
    let f: RandomAccessFile = new RandomAccessFileInner(0);
    callback(e, f);
  });
}

function createRandomAccessFile(file: string | File, mode: number,
  callback: AsyncCallback<RandomAccessFile, void>): void {
  let promise = taskpool.execute(FileIoImpl.createRandomAccessFileSync, file, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let raffile = ret as RandomAccessFile;
    callback(e, raffile);
  }).catch((e: BusinessError): void => {
    let f: RandomAccessFile = new RandomAccessFileInner(0);
    callback(e, f);
  });
}

function fdopenStream(fd: number, mode: string, callback: AsyncCallback<Stream>): void {
  let promise = taskpool.execute(FileIoImpl.fdopenStreamSync, fd, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let stream = ret as Stream;
    callback(e, stream);
  }).catch((e: BusinessError): void => {
    let r: Stream = new StreamInner(0);
    callback(e, r);
  });
}

function fdopenStreamSync(fd: number, mode: string): Stream {
  return FileIoImpl.fdopenStreamSync(fd, mode);
}

function createStream(path: string, mode: string): Promise<Stream> {
  return new Promise<Stream>((resolve: (result: Stream) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.createStreamSync, path, mode);
    promise.then((ret: NullishType): void => {
      let stream = ret as Stream;
      resolve(stream);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function createStream(path: string, mode: string, callback: AsyncCallback<Stream>): void {
  let promise = taskpool.execute(FileIoImpl.createStreamSync, path, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    let stream = ret as Stream;
    callback(e, stream);
  }).catch((e: BusinessError): void => {
    let r: Stream = new StreamInner(0);
    callback(e, r);
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

function createWatcher(path: string, events: number, listener: WatchEventListener): Watcher {
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
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function utimes(path: string, mtime: number): void {
  return FileIoImpl.utimes(path, mtime);
}

function lstatSync(path: string): Stat {
  return FileIoImpl.lstatSync(path)
}

function lstat(path: string): Promise<Stat> {
  return new Promise<Stat>((resolve: (result: Stat) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(FileIoImpl.lstatSync, path);
    promise.then((ret: NullishType): void => {
      if (ret === null || ret === undefined) {
        let e = new BusinessError<void>();
        e.code = -1;
        reject(e);
      } else {
        let r = ret as Stat;
        resolve(r);
      }
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function lstat(path: string, callback: AsyncCallback<Stat, void>): void {
  let p = taskpool.execute(FileIoImpl.lstatSync, path);
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
  }).catch((e: BusinessError): void => {
    callback(e, new StatInner(0));
  });
}
function copyFile(src: string | number, dest: string | number, mode?: number): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    if (mode === undefined) {
      let promise = taskpool.execute((src: string | number, dest: string | number): undefined =>
        FileIoImpl.copyFileSync(src, dest), src, dest);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((src: string | number, dest: string | number, mode: number): undefined =>
        FileIoImpl.copyFileSync(src, dest, mode), src, dest, mode);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function copy(srcUri: string, destUri: string, options?: CopyOptions): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    if (options === undefined) {
      let promise = taskpool.execute((srcUri: string, destUri: string): undefined =>
        FileIoImpl.copySync(srcUri, destUri), srcUri, destUri);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute((srcUri: string, destUri: string, options: CopyOptions): undefined =>
        FileIoImpl.copySync(srcUri, destUri, options), srcUri, destUri, options);
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
  });
}

function copyFile(src: string | number, dest: string | number, mode: number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string | number, dest: string | number, mode: number): undefined =>
    FileIoImpl.copyFileSync(src, dest, mode), src, dest, mode);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function copy(srcUri: string, destUri: string, options: CopyOptions, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((srcUri: string, destUri: string, options: CopyOptions): undefined =>
    FileIoImpl.copySync(srcUri, destUri, options), srcUri, destUri, options);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function copyFile(src: string | number, dest: string | number, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((src: string | number, dest: string | number): undefined =>
    FileIoImpl.copyFileSync(src, dest), src, dest);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function copy(srcUri: string, destUri: string, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((srcUri: string, destUri: string): undefined =>
    FileIoImpl.copySync(srcUri, destUri), srcUri, destUri);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function lseek(fd: number, offset: number, whence?: WhenceType): number {
  return FileIoImpl.lseekSync(fd, offset, whence);
}

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

export interface Filter {
  suffix?: Array<string>;
  displayName?: Array<string>;
  mimeType?: Array<string>;
  fileSizeOver?: number;
  lastModifiedAfter?: number;
  excludeMedia?: boolean;
}

export interface Progress {
  processedSize: number;
  totalSize: number;
}

export interface DfsListeners {
  onStatus(networkId: string, status: number): void;
}

export class ProgressInner implements Progress {
  processedSize: number;
  totalSize: number;

  constructor(pSize: number, tSize: number) {
    this.processedSize = pSize;
    this.totalSize = tSize;
  }
}

export type ProgressListener = (progress: Progress) => void;

export class TaskSignal {
  private nativeTaskSignal: long = 0;
  private native onCancelNative(): void;
  private onCancelResolve: (path: string) => void = (path: string): void => {};
  private onCancelCallback(path: string): void {
    if (this.onCancelResolve) {
      this.onCancelResolve(path);
    }
  }
  native cancel(): void;
  onCancel(): Promise<string> {
    return new Promise<string>((resolve: (path: string) => void, reject: (e: BusinessError<void>) => void): void => {
      this.onCancelResolve = resolve;
      this.onCancelNative();
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
  fd: number;
  filePointer: number;

  setFilePointer(filePointer: number): void;
  close(): void;
  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<number>;
  write(buffer: ArrayBuffer | string, callback: AsyncCallback<number>): void;
  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<number>): void;
  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): number;
  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<number>;
  read(buffer: ArrayBuffer, callback: AsyncCallback<number>): void;
  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<number>): void;
  readSync(buffer: ArrayBuffer, options?: ReadOptions): number;
  getReadStream(): ReadStream;
  getWriteStream(): WriteStream;
}

export class RandomAccessFileInner implements RandomAccessFile {
  fd: number = -1;
  filePointer: number = -1;

  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  setFilePointer(filePointer: number): void {
    this.setFilePointer0(filePointer);
    this.filePointer = filePointer;
  }

  native setFilePointer0(filePointer: number): void;
  
  native close(): void;

  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): number {
    let length = options ? this.writeSync0(buffer, options) : this.writeSync0(buffer);
    this.filePointer += length;
    return length;
  }

  native writeSync0(buffer: ArrayBuffer | string, options?: WriteOptions): number;

  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
      if (options === undefined) {
        let promise = taskpool.execute(this.writeSync, buffer, options);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      } else {
        let promise = taskpool.execute(this.writeSync, buffer, options);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      }
    });
  }

  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<number, void>): void {
    let promise = taskpool.execute(this.writeSync, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  write(buffer: ArrayBuffer | string, callback: AsyncCallback<number, void>): void {
    let promise = taskpool.execute(this.writeSync, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  readSync(buffer: ArrayBuffer, options?: ReadOptions): number {
    const length = options ? this.readSync0(buffer, options) : this.readSync0(buffer);
    this.filePointer += length;
    return length;
  }

  native readSync0(buffer: ArrayBuffer, options?: ReadOptions): number;

  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
      if (options === undefined) {
        let promise = taskpool.execute(this.readSync, buffer);
        promise.then((ret: NullishType): void => {
          let result = ret as number;
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      } else {
        let promise = taskpool.execute(this.readSync, buffer, options);
        promise.then((ret: NullishType): void => {
          let result = ret as number;
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      }
    });
  }

  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<number, void>): void {
    let promise = taskpool.execute(this.readSync, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  read(buffer: ArrayBuffer, callback: AsyncCallback<number, void>): void {
    let promise = taskpool.execute(this.readSync, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  native getReadStream(): ReadStream;
  native getWriteStream(): WriteStream;
}

export interface File {
  fd: number;
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
  fd: number = -1;
  path: String = "";
  name: String = "";

  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  native getParent(): String;
  native lockSync(exclusive?: boolean): void;

  lock(exclusive?: boolean): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      if (exclusive === undefined) {
        let promise = taskpool.execute((): undefined => {
          return this.lockSync();
        });
        promise.then((ret: NullishType): void => {
          resolve(undefined);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      } else {
        let promise = taskpool.execute((exclusive: boolean): undefined => {
          return this.lockSync(exclusive);
        }, exclusive);
        promise.then((ret: NullishType): void => {
          resolve(undefined);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      }
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
    }).catch((e: BusinessError): void => {
      callback(e, undefined);
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
    }).catch((e: BusinessError): void => {
      callback(e, undefined);
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
  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  done: boolean = false;
  value: string = "";
}

export interface ReaderIterator {
  next(): ReaderIteratorResult;
}

export class ReaderIteratorInner implements ReaderIterator {
  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  native next(): ReaderIteratorResult;
}

export interface Stat {
  ino: bigint;
  mode: number;
  uid: number;
  gid: number;
  size: number;
  atime: number;
  mtime: number;
  ctime: number;
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
  mode: number;
  uid: number;
  gid: number;
  size: number;
  atime: number;
  mtime: number;
  ctime: number;
  atimeNs: bigint = 0n;
  mtimeNs: bigint = 0n;
  ctimeNs: bigint = 0n;
  location: LocationType = LocationType.LOCAL;

  private nativeStat: long = 0;

  constructor(stat: long) {
    if (this.nativeStat === 0) {
      this.nativeStat = stat;
    }
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
  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<number>;
  write(buffer: ArrayBuffer | string, callback: AsyncCallback<number>): void;
  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<number>): void;
  writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): number;
  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<number>;
  read(buffer: ArrayBuffer, callback: AsyncCallback<number>): void;
  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<number>): void;
  readSync(buffer: ArrayBuffer, options?: ReadOptions): number;
  seek(offset: number, whence?: number): number;
}

export class StreamInner implements Stream {
  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr === 0) {
      this.nativePtr = ptr;
    }
  }

  close(): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute((): undefined => this.closeSync());
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    });
  }

  close(callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((): undefined => this.closeSync());
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: BusinessError): void => {
      callback(e, undefined);
    });
  }

  native closeSync(): void;

  flush(): Promise<void> {
    return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute((): undefined => this.flushSync());
      promise.then((ret: NullishType): void => {
        resolve(undefined);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    });
  }

  flush(callback: AsyncCallback<void>): void {
    let promise = taskpool.execute((): undefined => this.flushSync());
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      callback(e, undefined);
    }).catch((e: BusinessError): void => {
      callback(e, undefined);
    });
  }

  native flushSync(): void;

  write(buffer: ArrayBuffer | string, options?: WriteOptions): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
      if (options === undefined) {
        let promise = taskpool.execute(this.writeSync, buffer);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      } else {
        let promise = taskpool.execute(this.writeSync, buffer, options);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      }
    });
  }

  write(buffer: ArrayBuffer | string, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(this.writeSync, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  write(buffer: ArrayBuffer | string, options: WriteOptions, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(this.writeSync, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  native writeSync(buffer: ArrayBuffer | string, options?: WriteOptions): number;

  read(buffer: ArrayBuffer, options?: ReadOptions): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void) => {
      if (options === undefined) {
        let promise = taskpool.execute(this.readSync, buffer);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      } else {
        let promise = taskpool.execute(this.readSync, buffer, options);
        promise.then((ret: NullishType): void => {
          let result = ret as number
          resolve(result);
        }).catch((e: BusinessError): void => {
          reject(e);
        });
      }
    });
  }

  read(buffer: ArrayBuffer, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(this.readSync, buffer);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }

  read(buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(this.readSync, buffer, options);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: BusinessError): void => {
      callback(e, 0);
    });
  }
  
  native readSync(buffer: ArrayBuffer, options?: ReadOptions): number;
  native seek(offset: number, whence?: number): number;
}

export class ReadStream extends stream.Readable {
  path: string;
  bytesRead: number;
  private offset: number;
  private start?: number;
  private end?: number;
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

  seek(offset: number, whence?: WhenceType): number {
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

  doInitialize(callback: () => void): void {
    callback();
  }

  doRead(size: number): void {
    let readSize = size;
    let end = this.end
    if (end !== undefined) {
      if (this.offset > end) {
        this.push(null);
        return;
      }
      if (this.offset + readSize > end) {
        readSize = end - this.offset;
      }
    }
    let buffer = new ArrayBuffer(readSize);
    const off = this.offset;
    this.offset += readSize;
    this.stream?.read(buffer, { offset: off, length: readSize })
      .then((readOut: number) => {
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
  bytesWritten: number;
  private offset: number;
  private mode: string;
  private start?: number;
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

  seek(offset: number, whence?: WhenceType): number {
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

  doInitialize(callback: () => void): void {
    callback();
  }

  doWrite(chunk: string | ArrayBuffer, encoding: string, callback: () => void): void {
    this.stream?.write(chunk, { offset: this.offset })
      .then((writeIn: number) => {
        this.offset += writeIn;
        this.bytesWritten += writeIn;
        callback();
      })
      .finally(() => {
        this.stream?.flush();
      });
  }

  convertOpenMode(mode?: number): string {
    let modeStr = 'w';
    if (mode === undefined) {
      return modeStr;
    }
    if ((mode as number) & fileIo.OpenMode.WRITE_ONLY) {
      modeStr = 'w';
    }
    if ((mode as number) & fileIo.OpenMode.READ_WRITE) {
      modeStr = 'w+';
    }
    if (((mode as number) & fileIo.OpenMode.WRITE_ONLY) && ((mode as number) & fileIo.OpenMode.APPEND)) {
      modeStr = 'a';
    }
    if (((mode as number) & fileIo.OpenMode.READ_WRITE) && ((mode as number) & fileIo.OpenMode.APPEND)) {
      modeStr = 'a+';
    }
    return modeStr;
  }
}

export class AtomicFile {
  static {
    loadLibrary("ani_file_fs");
  }

  private nativePtr: long = 0;
  private native getPath(): string;
  private writeStream: WriteStream;

  native constructor(path: string);

  native getBaseFile(): File;

  native openRead(): ReadStream;

  native readFully(): ArrayBuffer;

  native nativeStartWrite(): WriteStream;
  startWrite(): WriteStream {
    let ws = this.nativeStartWrite();
    this.writeStream = ws;
    return this.writeStream;
  }

  native nativeFinishWrite(): void;
  finishWrite(): void {
    if (!this.writeStream) {
      throw createBusinessError(UNKNOWN_ERR, UNKNOWN_MSG);
    }
    this.writeStream.close();
    this.nativeFinishWrite();
  };

  native nativeFailWrite(): void;
  failWrite(): void {
    if (!this.writeStream) {
      throw createBusinessError(UNKNOWN_ERR, UNKNOWN_MSG);
    }
    this.writeStream.close();
    this.nativeFailWrite();
  };

  native delete(): void;
}

export enum WhenceType {
  SEEK_SET = 0,
  SEEK_CUR = 1,
  SEEK_END = 2
}

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

export interface WatchEvent {
  fileName: string;
  event: number;
  cookie: number;
}

class WatchEventInner implements WatchEvent {
  fileName: string = '';
  event: number;
  cookie: number;

  constructor(fileName: string, event: number, cookie: number) {
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
  offset?: number;
  length?: number;
}

export interface ReadTextOptions extends ReadOptions {
  encoding?: string;
}

export interface WriteOptions extends Options {
  offset?: number;
  length?: number;
}

export interface RandomAccessFileOptions {
  start?: number;
  end?: number;
}

export interface ListFileOptions {
  recursion?: boolean;
  listNum?: number;
  filter?: Filter;
}

export interface WriteStreamOptions {
  mode?: number;
  start?: number;
}

export interface ReadStreamOptions {
  start?: number;
  end?: number;
}

export class ReadStreamOptionsInner implements ReadStreamOptions {
  constructor() {}
  start?: number;
  end?: number;
}

export class WriteStreamOptionsInner implements WriteStreamOptions {
  constructor() {}
  mode?: number;
  start?: number;
}

export interface ReaderIteratorResult {
  done: boolean;
  value: string;
}

export interface Filter {
  suffix?: Array<string>;
  displayName?: Array<string>;
  mimeType?: Array<string>;
  fileSizeOver?: number;
  lastModifiedAfter?: number;
  excludeMedia?: boolean;
}

type TaskSignal = fileIo.TaskSignal;
type DfsListeners = fileIo.DfsListeners;


export class FileIoImpl {

  static {
    loadLibrary("ani_file_fs");
  }

  static native doAccessSync(path: string, mode?: fileIo.AccessModeType, flag?: fileIo.AccessFlagType): boolean;

  static native closeSync(file: number | fileIo.File): void;

  static native copySync(srcUri: string, destUri: string, options?: fileIo.CopyOptions): void;

  static native connectDfs(networkId: string, listeners: DfsListeners): void;

  static native copyDirSync(src: string, dest: string, mode?: number): void;

  static native copyFileSync(src: string | number, dest: string | number, mode?: number): void;

  static native disConnectDfs(networkId: string): void;

  static native fdatasyncSync(fd: number): void;

  static native getxattrSync(path: string, key: string): string;

  static native createStreamSync(path: string, mode: string): fileIo.Stream;

  static native createWatcherSync(path: string, events: number, listener: WatchEventListener): Watcher;
  
  static native fdopenStreamSync(fd: number, mode: string): fileIo.Stream;
  
  static native dup(fd: number): fileIo.File;

  static native listFileSync(path: string, options?: ListFileOptions): string[];

  static native lstatSync(path: string): fileIo.Stat;
  
  static native lseekSync(fd: number, offset: number, whence?: fileIo.WhenceType): number;

  static native mkdirSync(path: string): void;

  static native mkdirSync(path: string, recursion: boolean): void;

  static native movedirSync(src: string, dest: string, mode?: number): void;

  static native mkdtempSync(prefix: string): string;

  static native moveFileSync(src: String, dest: String, mode?: number): void;

  static native openSync(path: String, mode?: number): fileIo.File;

  static native readlinesSync(filePath: string, options?: Options): fileIo.ReaderIterator;
  
  static native readSync(fd: number, buffer: ArrayBuffer, options?: ReadOptions): number;

  static native readTextSync(filePath: string, options?: ReadTextOptions): string;

  static native rmdirSync(path: string): void;

  static native setxattrSync(path: string, key: string, value: string): void;

  static native statSync(file: string | number): fileIo.Stat;

  static native truncateSync(file: string | number, len?: number): void;

  static native unlinkSync(path: string): void;

  static native writeSync(fd: number, buffer: string | ArrayBuffer, options?: WriteOptions): number;

  static native fsyncSync(fd: number): void;

  static native renameSync(oldPath: string, newPath: string): void;

  static native createRandomAccessFileSync(file: string | fileIo.File, mode?: number,
    options?: RandomAccessFileOptions): fileIo.RandomAccessFile;

  static native symlinkSync(target: string, srcPath: string): void;

  static native utimes(path: string, mtime: number): void;
}
