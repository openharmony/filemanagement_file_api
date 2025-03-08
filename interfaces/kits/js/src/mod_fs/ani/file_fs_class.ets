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


enum AccessModeType {
  EXIST = 0,

  WRITE = 2,

  READ = 4,

  READ_WRITE = 6,
}

enum AccessFlagType {
  LOCAL = 0,
}


class fileIo {

  static native doAccessSync(path:string, mode?:AccessModeType, flag?:AccessFlagType):boolean;
  static native unlinkSync(path: string): int;
  static native mkdirSync(path: string): int;
  static native mkdirSync(path: string, recursion: boolean): int;
  static native moveFileSync(src: String, dest: String, mode?: int): void;
  static native rmdirSync(path: string): void;

  static accessSync(path: string, mode?: AccessModeType): boolean {
      return fileIo.doAccessSync(path, mode);
  }

  static accessSync(path: string, mode: AccessModeType, flag: AccessFlagType): boolean{
      return fileIo.doAccessSync(path, mode, flag);
  }

  static accessSync1(path: string):boolean {
      return fileIo.accessSync(path);
  }

  static accessSync2(path: string, mode: AccessModeType):boolean {
      return fileIo.accessSync(path, mode);
  }

  static accessSync3(path: string, mode: AccessModeType, flag: AccessFlagType): boolean
  {
      return fileIo.accessSync(path, mode, flag);
  }

  static access(path: string, mode?: AccessModeType): Promise<boolean> {
      return new Promise<boolean>((resolve:(result:boolean)=>void, reject:(e:BusinessError<void>)=>void) => {
          if (mode === undefined) {
              let promise = taskpool.execute(fileIo.accessSync1, path); // 这里调用同步方法
              promise.then((ret:NullishType) => {
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
              promise.then((ret:NullishType) => {
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

  static access(path: string, mode: AccessModeType, flag: AccessFlagType): Promise<boolean>{
      return new Promise<boolean>((resolve:(result:boolean)=>void, reject:(e:BusinessError<void>)=>void) => {
          let promise = taskpool.execute(fileIo.accessSync3, path, mode, flag); // 这里调用同步方法
          promise.then((ret:NullishType) => {
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

  static access(path: string, callback: AsyncCallback<boolean,void>): void {
      let promise = taskpool.execute(fileIo.accessSync1, path); // 这里调用同步方法
      promise.then((ret:NullishType) => {
          let err = new BusinessError<void>();
          if (ret === null || ret === undefined) { // 异常处理
              err.code = -1;
              callback(err,undefined);
          } else {
              err.code = 0;
              let result = ret as boolean;
              callback(err,result); // 正常结果
          }
      });
  }

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
        }else {
          console.println("-------- err code = 0 -------------");
          err.code = 0
          let r =  ret as int;
          callback(err, r);
        }
    });
  }

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
        }else {
          console.println("-------- err code = 0 -------------");
          err.code = 0
          let r =  ret as int;
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
        }else {
          console.println("-------- err code = 0 -------------");
          err.code = 0
          let r =  ret as int;
          callback(err, r);
        }
    });
  }

  static native copyFileSync(src: FileDataType, dest: FileDataType, mode?: int): void;
  static native statSync(file: FileDataType): Stat;
}

type FileDataType = String | number

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

  private nativeStat:long = 0;
  
  constructor(stat:long) {
      if(this.nativeStat == 0){
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


function accessSyncTest() {
  let file_list = ['/data/zxl/test/not_read_and_write.txt','/data/zxl/test/read.txt',
    '/data/zxl/test/read_and_write.txt','/data/zxl/test/write.txt', '/data/zxl/test/no.txt']

  for (let filepath of file_list) {
    console.info('test file name: ',filepath, ' start:')
    let result1 = fileIo.accessSync(filepath, AccessModeType.EXIST, AccessFlagType.LOCAL)
    console.info('access EXIST', result1)
    let result2 = fileIo.accessSync(filepath, AccessModeType.WRITE, AccessFlagType.LOCAL)
    console.info('access WRITE', result2)
    let result3 = fileIo.accessSync(filepath, AccessModeType.READ, AccessFlagType.LOCAL)
    console.info('access READ', result3)
    let result4 = fileIo.accessSync(filepath, AccessModeType.READ_WRITE, AccessFlagType.LOCAL)
    console.info('access READ_WRITE', result4) 
  }
}

function unlinkTest() {
  console.println("unlinkTest begin")
  const ret = fileIo.unlinkSync("liyuke")
  console.println("------- end --------")
  console.println(ret)
  // promise异步
  try {
      fileIo.unlink("src").then((result: int) => {
      console.println("------- end --------")
      console.println(result)
    });;
  } catch (error) {
    console.error("Promise: Error getting temp dir:", error);
  }
  // callback异步
  let path = "/data/local/tmp/hello.txt";
  fileIo.unlink(path, (err: BusinessError<void>, data?: int) => {
    console.println("-------- in callback -------------");
    if (err.code) {
      console.error("Callback方式: Error unlink:", err);
    } else {
      console.log("Callback方式: unlink:", data);
    }
  });
  console.println("unlinkTest end")
}

function mkdirTest() {
  console.println("mkdirTest begin")
  const ret = fileIo.mkdirSync("liyuke")
  console.println("------- end --------")
  console.println(ret)
  // promise异步
  try {
      fileIo.mkdir("src").then((result: int) => {
      console.println("------- end --------")
      console.println(result)
    });;
  } catch (error) {
    console.error("Promise: Error getting temp dir:", error);
  }

  try {
      fileIo.mkdir("test/111", true).then((result: int) => {
      console.println("------- end --------")
      console.println(result)
    });;
  } catch (error) {
    console.error("Promise: Error getting temp dir:", error);
  }
  try {
    fileIo.mkdir("test1/222", false).then((result: int) => {
    console.println("------- end --------")
    console.println(result)
  });;
} catch (error) {
  console.error("Promise: Error getting temp dir:", error);
}
  // callback异步
  let path = "/data/local/tmp/hello";
  fileIo.mkdir(path, (err: BusinessError<void>, data?: int) => {
    console.println("-------- in callback -------------");
    if (err.code) {
      console.error("Callback方式: Error mkdir:", err);
    } else {
      console.log("Callback方式: mkdir:", data);
    }
  });

  path = "/data/local/tmp/111/test";
  let recursion = true;
  fileIo.mkdir(path, recursion, (err: BusinessError<void>, data?: int) => {
    console.println("-------- in callback -------------");
    if (err.code) {
      console.error("Callback方式: Error mkdir:", err);
    } else {
      console.log("Callback方式: mkdir:", data);
    }
  });
  path = "/data/local/tmp/222/test";
  recursion = false;
  fileIo.mkdir(path, recursion, (err: BusinessError<void>, data?: int) => {
    console.println("-------- in callback -------------");
    if (err.code) {
      console.error("Callback方式: Error mkdir:", err);
    } else {
      console.log("Callback方式: mkdir:", data);
    }
  });
  console.println("mkdirTest end")
}

function moveSyncTest() {
  console.println("moveSyncTest begin")
  fileIo.moveFileSync("/data/local/tmp/a.txt", "/data/local/tmp/b.txt", 1)
  console.println("moveSyncTest end")
}

function copy_file_test(src: FileDataType, dest: FileDataType, mode?: int) {
  fileIo.copyFileSync(src, dest, mode)
}

function rmdirSyncTest() {
  console.println("rmdirSyncTest begin")
  fileIo.rmdirSync("/data/local/hello")
  console.println("rmdirSyncTest end")
}

function stat_test(file: FileDataType) {
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

function main(){
  console.println("---------- hello ani --------------")
  accessSyncTest()
  moveSyncTest()
  rmdirSyncTest()
  console.println("---------- hello ani end ---------------")
}