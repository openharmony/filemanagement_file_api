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

loadLibrary("ani_fs_class.z")

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
  LOCAl = 1,
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
  location: LocationType;

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

class fileIo {
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

}

function closeSyncTest1() {
  console.println("closeSyncTest1 begin");
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt");
    console.println(`open file without mode, fd=${file.fd}, path=${file.path}, name=${file.name}`);
    let ret = fileIo.closeSync(file.fd);
    console.println(`close file by fd, fd=${file.fd}, ret=${ret}`);
  } catch (error) {
    console.error("closeSyncTest1 Error!", error);
  }
  console.println("closeSyncTest1 end");
}

function closeSyncTest2() {
  console.println("closeSyncTest2 begin");
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt", 2);
    console.println(`open file with mode, fd=${file.fd}, path=${file.path}, name=${file.name}`);
    let ret = fileIo.closeSync(file);
    console.println(`close file by file, fd=${file.fd}, ret=${ret}`);
  } catch (error) {
    console.error("closeSyncTest2 Error!", error);
  }
  console.println("closeSyncTest2 end");
}

function copyFileTest() {
  console.println("copyFileTest begin");
  try {
    fileIo.copyFileSync("/data/local/tmp/a.txt", "/data/local/tmp/b.txt");
  } catch (error) {
    console.error("copyFileTest Error!", error);
  }
  console.println("copyFileTest end");
}

function listFileTest() {
  console.println("listFileTest begin");
  try {
    let files = fileIo.listFileSync("/data/local/tmp/");
    for (const file of files) {
      console.println(file);
    }
  } catch (error) {
    console.error("listFileTest Error!", error);
  }
  console.println("listFileTest end");
}

function mkdirTest1() {
  console.println("mkdirTest1 begin")
  try {
    const ret = fileIo.mkdirSync("/data/local/tmp/dir01");
    console.println(`mkdir result: ${ret}`);
  } catch (error) {
    console.error("mkdirTest1 Error!", error);
  }
  console.println("mkdirTest1 end");
}

function moveSyncTest() {
  console.println("moveSyncTest begin")
  fileIo.moveFileSync("/data/local/tmp/a.txt", "/data/local/tmp/a1.txt", 1)
  console.println("moveSyncTest end")
}

function openSyncTest() {
  console.println("openSyncTest begin");
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt");
    console.println(`open file without mode, fd=${file.fd}, path=${file.path}, name=${file.name}`);
    let mode = 2;
    file = fileIo.openSync("/data/local/tmp/a.txt", mode);
    console.println(`open file with mode=${mode}, fd=${file.fd}, path=${file.path}, name=${file.name}`);
  } catch (error) {
    console.error("openSyncTest Error!", error);
  }
  console.println("openSyncTest end");
}

function readSyncTest1() {
  console.println("readSyncTest1 begin")
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt")
    let fd = file.fd;
    console.println(`open file, fd=${fd}`)
    let buffer = new ArrayBuffer(100);
    let length = fileIo.readSync(fd, buffer);
    console.println(`read length: ${length}`)
    let len = length as int;
    let content = ArrayBuffer.stringify(buffer, "utf-8", 0, len);
    console.println("-----------------------------------------------")
    console.println(content)
    console.println("-----------------------------------------------")
  } catch (error) {
    console.error("readSyncTest1 Error!", error);
  }
  console.println("readSyncTest1 end")
}

function readSyncTest2() {
  console.println("readSyncTest2 begin")
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt")
    let fd = file.fd;
    console.println(`open file, fd=${fd}`)
    let buffer = new ArrayBuffer(100);
    let options: ReadOptions = {
      length: 5,
    }
    let length = fileIo.readSync(fd, buffer, options);
    console.println(`read length: ${length}`)
    let len = length as int;
    let content = ArrayBuffer.stringify(buffer, "utf-8", 0, len);
    console.println("-----------------------------------------------")
    console.println(content)
    console.println("-----------------------------------------------")
  } catch (error) {
    console.error("readSyncTest2 Error!", error);
  }
  console.println("readSyncTest2 end")
}

function readSyncTest3() {
  console.println("readSyncTest3 begin")
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt")
    let fd = file.fd;
    console.println(`open file, fd=${fd}`)
    let buffer = new ArrayBuffer(100);
    let options: ReadOptions = {
      offset: 3,
      length: 5,
    }
    let length = fileIo.readSync(fd, buffer, options);
    console.println(`read length: ${length}`)
    let len = length as int;
    let content = ArrayBuffer.stringify(buffer, "utf-8", 0, len);
    console.println("-----------------------------------------------")
    console.println(content)
    console.println("-----------------------------------------------")
  } catch (error) {
    console.error("readSyncTest3 Error!", error);
  }
  console.println("readSyncTest3 end")
}

function readTextSyncTest1() {
  console.println("readTextSyncTest1 begin")
  try {
    let content = fileIo.readTextSync("/data/local/tmp/a.txt");
    console.println("-----------------------------------------------")
    console.println(content)
    console.println("-----------------------------------------------")
  } catch (error) {
    console.error("readTextSyncTest1 Error!", error);
  }
  console.println("readTextSyncTest1 end")
}

function readTextSyncTest2() {
  console.println("readTextSyncTest2 begin");
  try {
    let options: ReadTextOptions = {
      length: 5,
    }
    let content = fileIo.readTextSync("/data/local/tmp/a.txt", options);
    console.println("-----------------------------------------------");
    console.println(content);
    console.println("-----------------------------------------------");
  } catch (error) {
    console.error("readTextSyncTest2 Error!", error);
  }
  console.println("readTextSyncTest2 end");
}

function readTextSyncTest3() {
  console.println("readTextSyncTest3 begin")
  try {
    let options: ReadTextOptions = {
      offset: 3,
      length: 5,
    }
    let content = fileIo.readTextSync("/data/local/tmp/a.txt", options);
    console.println("-----------------------------------------------");
    console.println(content);
    console.println("-----------------------------------------------");
  } catch (error) {
    console.error("readTextSyncTest3 Error!", error);
  }
  console.println("readTextSyncTest3 end");
}

function rmdirSyncTest() {
  console.println("rmdirSyncTest begin");
  try {
    fileIo.rmdirSync("/data/local/tmp/dir01");
  } catch (error) {
    console.error("rmdirSyncTest Error!", error);
  }
  console.println("rmdirSyncTest end");
}

function statSyncTest(file: PathOrFd) {
  let stat = fileIo.statSync(file)
  console.info("stat, ino is " + stat.ino);
  console.info("stat, mode is " + stat.mode);
  console.info("stat, uid is " + stat.uid);
  console.info("stat, gid is " + stat.gid);
  console.info("stat, size is " + stat.size);
  console.info("stat, atime is " + stat.atime);
  console.info("stat, mtime is " + stat.mtime);
  console.info("stat, ctime is " + stat.ctime);
  console.info("stat, atimeNs is " + stat.atimeNs);
  console.info("stat, mtimeNs is " + stat.mtimeNs);
  console.info("stat, ctimeNs is " + stat.ctimeNs);
  console.info("stat, location is " + stat.location);
  console.info("stat, isBlockDevice is " + stat.isBlockDevice());
  console.info("stat, isCharacterDevice is " + stat.isCharacterDevice());
  console.info("stat, isDirectory is " + stat.isDirectory());
  console.info("stat, isFIFO is " + stat.isFIFO());
  console.info("stat, isFile is " + stat.isFile());
  console.info("stat, isSocket is " + stat.isSocket());
  console.info("stat, isSymbolicLink is " + stat.isSymbolicLink());
}

function statSyncTest1() {
  console.println("statSyncTest1 begin");
  try {
    statSyncTest("/data/local/tmp/a.txt");
  } catch (error) {
    console.error("statSyncTest1 Error!", error);
  }
  console.println("statSyncTest1 end");
}

function statSyncTest2() {
  console.println("statSyncTest2 begin");
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt");
    console.println(`open file fd: ${file.fd}`);
    statSyncTest(file.fd);
  } catch (error) {
    console.error("statSyncTest2 Error!", error);
  }
  console.println("statSyncTest2 end");
}

function truncateSyncTest1() {
  console.println("truncateSyncTest1 begin");
  try {
    fileIo.truncateSync(-1, 4);
  } catch (error) {
    console.error("truncateSyncTest1 Error!", error);
  }
  console.println("truncateSyncTest1 end");
}

function truncateSyncTest2() {
  console.println("truncateSyncTest2 begin");
  try {
    let file = fileIo.openSync("/data/local/tmp/a.txt", 2);
    console.println(`open file, fd=${file.fd}`);
    fileIo.truncateSync(file.fd, 4);
  } catch (error) {
    console.error("truncateSyncTest2 Error!", error);
  }
  console.println("truncateSyncTest2 end");
}

function truncateSyncTest3() {
  console.println("truncateSyncTest3 begin");
  try {
    fileIo.truncateSync("/data/local/tmp/b.txt", 4);
  } catch (error) {
    console.error("truncateSyncTest3 Error!", error);
  }
  console.println("truncateSyncTest3 end");
}

function unlinkSyncTest() {
  console.println("unlinkSyncTest begin");
  try {
    const ret = fileIo.unlinkSync("/data/local/tmp/b.txt");
    console.println(`unlink result: ${ret}`);

  } catch (error) {
    console.error("unlinkSyncTest Error!", error);
  }
  console.println("unlinkSyncTest end");
}


function main() {
  console.println("---------- hello ani --------------");
  openSyncTest();
  closeSyncTest1();
  closeSyncTest2();
  copyFileTest();
  listFileTest();
  mkdirTest1();
  moveSyncTest();
  readSyncTest1();
  readSyncTest2();
  readSyncTest3();
  readTextSyncTest1();
  readTextSyncTest2();
  readTextSyncTest3();
  rmdirSyncTest();
  statSyncTest1();
  statSyncTest2();
  truncateSyncTest1();
  truncateSyncTest2();
  truncateSyncTest3();
  unlinkSyncTest();

  console.println("---------- hello ani end ---------------");
}