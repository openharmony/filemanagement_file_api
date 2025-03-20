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

function hashSync(path: string, algorithm: string): string {
  return HashImpl.hashSync(path, algorithm);
}

function hash(path: string, algorithm: string): Promise<string> {
  return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void) => {
    let promise = taskpool.execute(HashImpl.hashSync, path, algorithm);
    promise.then((ret: NullishType): void => {
      let res = ret as string;
      resolve(res);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function hash(path: string, algorithm: string, callback: AsyncCallback<string, void>): void {
  let promise = taskpool.execute(HashImpl.hashSync, path, algorithm);
  promise.then((ret: NullishType) => {
    let e = new BusinessError<void>();
    e.code = 0;
    let res = ret as string;
    callback(e, res);
  }).catch((e: BusinessError): void => {
    callback(e, "");
  });
}

class HashImpl {

  static {
    loadLibrary("ani_hash_class");
  }

  static native hashSync(path: string, algorithm: string): string;
}
