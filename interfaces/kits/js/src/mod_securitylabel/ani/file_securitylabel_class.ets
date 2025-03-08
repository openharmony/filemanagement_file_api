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

export default securitylabel;

loadLibrary("ani_securitylabel_class.z")

export class BusinessError<T = void> {
  code: number = 0;
  data?: T;
}

export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data?: T) => void;

type DataLevel = 's0' | 's1' | 's2' | 's3' | 's4';

class securitylabel {
  static native setSecurityLabelSync(path: string, type: DataLevel): int;

  // setSecurityLabel promise异步
  static setSecurityLabel(path: string, type: DataLevel): Promise<int> {
    return new Promise<int>((resolve:(result:int)=>void, reject:(e:BusinessError<void>)=>void) => {
      let promise = taskpool.execute(securitylabel.setSecurityLabelSync, path, type); // 这里调用同步方法
      promise.then((ret:NullishType) => {
        if (ret === null || ret === undefined) { // 异常处理
          let err = new BusinessError<void>();
          err.code = -1;
          reject(err);
        } else {
          let r =  ret as int;
          resolve(r); // 正常结果
        }
      });
    });
  }

  // setSecurityLabel callback异步
  static setSecurityLabel(path: string, type: DataLevel, callback: AsyncCallback<int, void>): void {
    let p1 = taskpool.execute(securitylabel.setSecurityLabelSync, path, type);
    p1.then((ret: NullishType) => {
      let err = new BusinessError<void>();
      if (ret === null || ret === undefined) {
        err.code = -1
        callback(err, undefined)
      }else {
        err.code = 0
        let r =  ret as int;
        callback(err, r);
      }
    });
  }
}

function setSecuritylabelTest() {
  let dataType: DataLevel = "s2"
  securitylabel.setSecurityLabelSync("/data/local/tmp/hello.txt", dataType)

  // promise异步
  dataType = "s3"
  try {
    securitylabel.setSecurityLabel("/data/local/tmp/hello.txt", dataType).then((result: int) => {
      console.println(result)
    });
  } catch (error) {
    console.error("Promise: Error listFile:", error);
  }
  console.println("listFile Promise async end");

  // callback异步
  dataType = "s4"
  securitylabel.setSecurityLabel("/data/local/tmp/hello.txt", dataType, (err: BusinessError<void>, data?: int) => {
    if (err.code) {
      console.error("Callback: Error listFile:", err);
    } else {
      console.log("Callback: listFile:", data);
    }
  });
  console.println("readText callback async end");
}

function main(){
    console.println("hello ani")
    setSecuritylabelTest()
    console.println("end ani")
}