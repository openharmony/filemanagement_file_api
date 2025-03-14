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

export default fileIo;

export class BusinessError<T = void> {
  code: number = 0;
  data?: T;
}

export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data?: T) => void;

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
  offset?: long;
  length?: long;
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

interface File {
  fd: int;
  path: String;
  name: String;

  getParent(): String;
  lock(exclusive?: boolean): void;
  tryLock(exclusive?: boolean): void;
  unlock(): void;
}

class FileInner implements File {
  fd: int = -1;
  path: String = "";
  name: String = "";

  private nativePtr: long = 0;

  constructor(ptr: long) {
    if (this.nativePtr == 0) {
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

interface Stat {
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
    if (this.nativeStat == 0) {
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

type FdOrFile = int | File;
type PathOrFd = string | int;
type BufferType = string | ArrayBuffer;

class fileIo {

  static {
    loadLibrary("ani_fs_class.z");
  }

  static native doAccessSync(path: string, mode?: AccessModeType, flag?: AccessFlagType): boolean;

  static accessSync(path: string, mode?: AccessModeType): boolean {
    return fileIo.doAccessSync(path, mode);
  }

  static accessSync(path: string, mode: AccessModeType, flag: AccessFlagType): boolean {
    return fileIo.doAccessSync(path, mode, flag);
  }

  static accessSync1(path: string): boolean {
    return fileIo.accessSync(path);
  }

  static accessSync2(path: string, mode: AccessModeType): boolean {
    return fileIo.accessSync(path, mode);
  }

  static accessSync3(path: string, mode: AccessModeType, flag: AccessFlagType): boolean {
    return fileIo.accessSync(path, mode, flag);
  }

  static access(path: string, mode?: AccessModeType): Promise<boolean> {
    return new Promise<boolean>((resolve: (result: boolean) => void, reject: (e: BusinessError<void>) => void) => {
      if (mode === undefined) {
        let promise = taskpool.execute(fileIo.accessSync1, path); // 这里调用同步方法
        promise.then((ret: NullishType) => {
          if (ret === null || ret === undefined) { // 异常处理
            let err = new BusinessError<void>();
            err.code = -1;
            reject(err);
          } else {
            let result = ret as boolean;
            resolve(result); // 正常结果
          }
        });
      } else {
        let promise = taskpool.execute(fileIo.accessSync2, path, mode); // 这里调用同步方法
        promise.then((ret: NullishType) => {
          if (ret === null || ret === undefined) { // 异常处理
            let err = new BusinessError<void>();
            err.code = -1;
            reject(err);
          } else {
            let result = ret as boolean;
            resolve(result); // 正常结果
          }
        });
      }
    });
  }

  static access(path: string, mode: AccessModeType, flag: AccessFlagType): Promise<boolean> {
    return new Promise<boolean>((resolve: (result: boolean) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.accessSync3, path, mode, flag); // 这里调用同步方法
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let result = ret as boolean;
          resolve(result); // 正常结果
        }
      });
    })
  }

  static access(path: string, callback: AsyncCallback<boolean, void>): void {
    let promise = taskpool.execute(fileIo.accessSync1, path); // 这里调用同步方法
    promise.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) { // 异常处理
        err.code = -1;
        callback(err, undefined);
      } else {
        err.code = 0;
        let result = ret as boolean;
        callback(err, result); // 正常结果
      }
    });
  }

  static native closeSync(file: FdOrFile): int;

  static native copyFileSync(src: PathOrFd, dest: PathOrFd, mode?: int): void;

  static native listFileSync(path: string, options?: ListFileOptions): string[];

  static listFileSync1(path: string): string[] {
    return fileIo.listFileSync(path);
  }
  static listFileSync2(path: string, options: ListFileOptions): string[] {
    return fileIo.listFileSync(path, options);
  }

  static listFile(path: string): Promise<string[]> {
    return new Promise<string[]>((resolve: (result: string[]) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.listFileSync1, path);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as string[];
          resolve(r);
        }
      });
    });
  }

  static listFile(path: string, options: ListFileOptions): Promise<string[]> {
    return new Promise<string[]>((resolve: (result: string[]) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.listFileSync2, path, options);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as string[];
          resolve(r);
        }
      });
    });
  }

  static listFile(path: string, callback: AsyncCallback<string[], void>): void {
    let p1 = taskpool.execute(fileIo.listFileSync1, path);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        err.code = -1;
        callback(err, undefined);
      } else {
        err.code = 0;
        let r = ret as string[];
        callback(err, r);
      }
    });
  }

  static listFile(path: string, options: ListFileOptions, callback: AsyncCallback<string[], void>): void {
    let p1 = taskpool.execute(fileIo.listFileSync2, path, options);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        err.code = -1;
        callback(err, undefined);
      } else {
        err.code = 0;
        let r = ret as string[];
        callback(err, r);
      }
    });
  }

  static native mkdirSync(path: string): int;

  static native mkdirSync(path: string, recursion: boolean): int;

  static mkdir(path: string): Promise<int> {
    return new Promise<int>((resolve: (result: int) => void, reject: (e: BusinessError<void>) => void) => {
      const mkdirSyncWrapper = (path: string) => fileIo.mkdirSync(path);
      let promise = taskpool.execute(mkdirSyncWrapper, path); // 这里调用同步方法
      promise.then((ret: NullishType) => {
        let result = ret as int
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          resolve(result); // 正常结果
        }
      });
    });
  }

  static mkdir(path: string, callback: AsyncCallback<int, void>): void {
    const mkdirSyncWrapper = (path: string) => fileIo.mkdirSync(path);
    let p1 = taskpool.execute(mkdirSyncWrapper, path);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1
        callback(err, undefined)
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0
        let r = ret as int;
        callback(err, r);
      }
    });
  }

  static mkdir(path: string, recursion: boolean): Promise<int> {
    return new Promise<int>((resolve: (result: int) => void, reject: (e: BusinessError<void>) => void) => {
      const mkdirSyncWrapper = (path: string, recursion: boolean) => fileIo.mkdirSync(path, recursion);
      let promise = taskpool.execute(mkdirSyncWrapper, path, recursion); // 这里调用同步方法
      promise.then((ret: NullishType) => {
        let result = ret as int
        if (ret === null || ret === undefined) { // 异常处理
          console.println("-------- err code = -1 -------------");
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          console.println("-------- err code = 0 -------------");
          resolve(result); // 正常结果
        }
      });
    });
  }

  static mkdir(path: string, recursion: boolean, callback: AsyncCallback<int, void>): void {
    const mkdirSyncWrapper = (path: string, recursion: boolean,) => fileIo.mkdirSync(path, recursion);
    let p1 = taskpool.execute(mkdirSyncWrapper, path, recursion);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1
        callback(err, undefined)
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0
        let r = ret as int;
        callback(err, r);
      }
    });
  }

  static native moveFileSync(src: String, dest: String, mode?: int): void;

  static native openSync(path: String, mode?: int): File;

  static openSync1(path: String, mode: int): File {
    return fileIo.openSync(path, mode);
  }
  static openSync2(path: String): File {
    return fileIo.openSync(path);
  }

  static open(path: String, mode: int): Promise<File> {
    return new Promise<File>((resolve: (result: File) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.openSync1, path, mode);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as File;
          resolve(r);
        }
      });
    });
  }

  static open(path: String): Promise<File> {
    return new Promise<File>((resolve: (result: File) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.openSync2, path);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as File;
          resolve(r);
        }
      });
    });
  }

  static open(path: String, mode: int, callback: AsyncCallback<File, void>): void {
    let p1 = taskpool.execute(fileIo.openSync1, path, mode);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as File;
        callback(err, r);
      }
    });
  }

  static open(path: String, callback: AsyncCallback<File, void>): void {
    let p1 = taskpool.execute(fileIo.openSync2, path);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as File;
        callback(err, r);
      }
    });
  }

  static native readSync(fd: int, buffer: ArrayBuffer, options?: ReadOptions): long;

  static readSync1(fd: int, buffer: ArrayBuffer): long {
    return fileIo.readSync(fd, buffer);
  }
  static readSync2(fd: int, buffer: ArrayBuffer, options: ReadOptions): long {
    return fileIo.readSync(fd, buffer, options);
  }

  static read(fd: int, buffer: ArrayBuffer): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.readSync1, fd, buffer);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as long;
          resolve(r);
        }
      });
    });
  }

  static read(fd: int, buffer: ArrayBuffer, options: ReadOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.readSync2, fd, buffer, options);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as long;
          resolve(r);
        }
      });
    });
  }

  static read(fd: int, buffer: ArrayBuffer, callback: AsyncCallback<long, void>): void {
    let p1 = taskpool.execute(fileIo.readSync1, fd, buffer);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as long;
        callback(err, r);
      }
    });
  }

  static read(fd: int, buffer: ArrayBuffer, options: ReadOptions, callback: AsyncCallback<long, void>): void {
    let p1 = taskpool.execute(fileIo.readSync2, fd, buffer, options);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as long;
        callback(err, r);
      }
    });
  }

  static native readTextSync(filePath: string, options?: ReadTextOptions): string;

  static readTextSync1(filePath: string): string {
    return fileIo.readTextSync(filePath);
  }
  static readTextSync2(filePath: string, options: ReadTextOptions): string {
    return fileIo.readTextSync(filePath, options);
  }

  static readText(filePath: string): Promise<string> {
    return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.readTextSync1, filePath);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as string;
          resolve(r); // 正常结果
        }
      });
    });
  }

  static readText(filePath: string, options: ReadTextOptions): Promise<string> {
    return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.readTextSync2, filePath, options);
      promise.then((ret: NullishType) => {
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r = ret as string;
          resolve(r); // 正常结果
        }
      });
    });
  }

  static readText(filePath: string, callback: AsyncCallback<string, void>): void {
    let p1 = taskpool.execute(fileIo.readTextSync1, filePath);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as string;
        callback(err, r);
      }
    });
  }

  static readText(filePath: string, options: ReadTextOptions, callback: AsyncCallback<string, void>): void {
    let p1 = taskpool.execute(fileIo.readTextSync2, filePath, options);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1;
        callback(err, undefined);
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0;
        let r = ret as string;
        callback(err, r);
      }
    });
  }

  static native rmdirSync(path: string): void;

  static native statSync(file: PathOrFd): Stat;

  static native truncateSync(file: PathOrFd, len?: long): void;

  static native unlinkSync(path: string): int;

  static unlink(path: string): Promise<int> {
    return new Promise<int>((resolve: (result: int) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.unlinkSync, path); // 这里调用同步方法
      promise.then((ret: NullishType) => {
        let result = ret as int
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          resolve(result); // 正常结果
        }
      });
    });
  }

  static unlink(path: string, callback: AsyncCallback<int, void>): void {
    let p1 = taskpool.execute(fileIo.unlinkSync, path);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1
        callback(err, undefined)
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0
        let r = ret as int;
        callback(err, r);
      }
    });
  }

  static native writeSync(fd: int, buffer: BufferType, options?: WriteOptions): long;

  static writeSync1(fd: int, buffer: BufferType, options: WriteOptions): long {
    return fileIo.writeSync(fd, buffer, options);
  }
  static writeSync2(fd: int, buffer: BufferType): long {
    return fileIo.writeSync(fd, buffer);
  }

  static write(fd: int, buffer: BufferType, options: WriteOptions): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.writeSync1, fd, buffer, options);
      promise.then((ret: NullishType) => {
        let result = ret as long
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          resolve(result);
        }
      });
    });
  }

  static write(fd: int, buffer: BufferType): Promise<long> {
    return new Promise<long>((resolve: (result: long) => void, reject: (e: BusinessError<void>) => void) => {
      let promise = taskpool.execute(fileIo.writeSync2, fd, buffer);
      promise.then((ret: NullishType) => {
        let result = ret as long
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          resolve(result);
        }
      });
    });
  }

  static write(fd: int, buffer: BufferType, options: WriteOptions, callback: AsyncCallback<long, void>): void {
    let p1 = taskpool.execute(fileIo.writeSync1, fd, buffer, options);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1
        callback(err, undefined)
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0
        let r = ret as long;
        callback(err, r);
      }
    });
  }

  static write(fd: int, buffer: BufferType, callback: AsyncCallback<long, void>): void {
    let p1 = taskpool.execute(fileIo.writeSync2, fd, buffer);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        console.println("-------- err code = -1 -------------");
        err.code = -1
        callback(err, undefined)
      } else {
        console.println("-------- err code = 0 -------------");
        err.code = 0
        let r = ret as long;
        callback(err, r);
      }
    });
  }
}
