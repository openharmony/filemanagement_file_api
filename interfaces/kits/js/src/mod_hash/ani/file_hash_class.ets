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

export default hash;

loadLibrary("ani_hash_class.z")

export class BusinessError<T = void> {
  code: number = 0;
  data?: T;
}

export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data?: T) => void;

class hash {
  static native hashSync(path: string, algorithm: string): string;

  // readText promise异步
  static hash(path: string, algorithm: string): Promise<string> {
    return new Promise<string>((resolve:(result:string)=>void, reject:(e:BusinessError<void>)=>void) => {
      let promise = taskpool.execute(hash.hashSync, path, algorithm);
      promise.then((ret:NullishType) => {
        if (ret === null || ret === undefined) {
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r =  ret as string;
          resolve(r);
        }
      });
    });
  }

  // readText callback异步
  static hash(path: string, algorithm: string, callback: AsyncCallback<string, void>): void {
    let p1 = taskpool.execute(hash.hashSync, path, algorithm);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        err.code = -1
        callback(err, undefined)
      }else {
        err.code = 0
        let r =  ret as string;
        callback(err, r);
      }
    });
  }
}

function hashTest() {
  let ret = hash.hashSync("/data/local/tmp/hello.txt", "sha256")
  console.println("hashSync end")
  console.println(ret)

  // promise异步
  try {
    hash.hash("/data/local/tmp/hello.txt", "sha256").then((result: string) => {
      console.println(result)
    });
  } catch (error) {
    console.error("Promise: Error getting temp dir:", error);
  }
  console.println("readText Promise async end");

  // callback异步
  hash.hash("/data/local/tmp/hello.txt", "sha256", (err: BusinessError<void>, data?: string) => {
    if (err.code) {
      console.error("Callback: Error hash:", err);
    } else {
      console.log("Callback: hash:", data);
    }
  });
  console.println("readText callback async end");
}

function main(){
    console.println("hello ani")
    hashTest()
    console.println("end ani")
}