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

function mkdirSync(path: string): void {
  return FileIoImpl.mkdirSync(path)
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

function moveFileSync(src: string, dest: string, mode?: number): void {
  return FileIoImpl.moveFileSync(src, dest, mode);
}

function openSync(path: string, mode?: number): File {
  return FileIoImpl.openSync(path, mode);
}

function open(path: String, mode?: number): Promise<File> {
  return new Promise<File>((resolve: (result: File) => void, reject: (e: BusinessError<void>) => void) => {
    if (mode === undefined) {
      let promise = taskpool.execute(FileIoImpl.openSync, path);
      promise.then((ret: NullishType): void => {
        let file = ret as File;
        resolve(file);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    } else {
      let promise = taskpool.execute(FileIoImpl.openSync, path, mode);
      promise.then((ret: NullishType): void => {
        let file = ret as File;
        resolve(file);
      }).catch((e: BusinessError): void => {
        reject(e);
      });
    }
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

function rmdirSync(path: string): void {
  return FileIoImpl.rmdirSync(path)
}

function truncateSync(file: string | number, len?: number): void {
  return FileIoImpl.truncateSync(file, len)
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

export interface Filter {
  suffix?: Array<string>;
  displayName?: Array<string>;
  mimeType?: Array<string>;
  fileSizeOver?: number;
  lastModifiedAfter?: number;
  excludeMedia?: boolean;
}

export interface ListFileOptions {
  recursion?: boolean;
  listNum?: number;
  filter?: Filter;
}

export interface ReadOptions {
  offset?: number;
  length?: number;
}

export interface ReadTextOptions extends ReadOptions {
  encoding?: string;
}

export interface WriteOptions {
  offset?: number;
  length?: number;
  encoding?: string;
}

enum AccessModeType {
  EXIST = 0,
  WRITE = 2,
  READ = 4,
  READ_WRITE = 6,
}

enum AccessFlagType {
  LOCAL = 0,
}

export interface File {
  fd: number;
  path: String;
  name: String;

  getParent(): String;
  lock(exclusive?: boolean): void;
  tryLock(exclusive?: boolean): void;
  unlock(): void;
}

class FileInner implements File {
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
  native lock(exclusive?: boolean): void;
  native tryLock(exclusive?: boolean): void;
  native unlock(): void;

}

enum LocationType {
  LOCAL = 1,
  CLOUD = 2
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

class StatInner implements Stat {
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

class FileIoImpl {

  static {
    loadLibrary("ani_fs_class");
  }

  static native doAccessSync(path: string, mode?: AccessModeType, flag?: AccessFlagType): boolean;

  static native closeSync(file: number | File): void;

  static native copyFileSync(src: string | number, dest: string | number, mode?: number): void;

  static native listFileSync(path: string, options?: ListFileOptions): string[];

  static native mkdirSync(path: string): void;

  static native mkdirSync(path: string, recursion: boolean): void;

  static native moveFileSync(src: String, dest: String, mode?: number): void;

  static native openSync(path: String, mode?: number): File;

  static native readSync(fd: number, buffer: ArrayBuffer, options?: ReadOptions): number;

  static native readTextSync(filePath: string, options?: ReadTextOptions): string;

  static native rmdirSync(path: string): void;

  static native statSync(file: string | number): Stat;

  static native truncateSync(file: string | number, len?: number): void;

  static native unlinkSync(path: string): void;

  static native writeSync(fd: number, buffer: string | ArrayBuffer, options?: WriteOptions): number;

}
