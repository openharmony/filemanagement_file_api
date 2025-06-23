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

namespace statfs {
  export function getFreeSizeSync(path: string): number {
    return StatvfsImpl.getFreeSizeSync(path);
  }

  export function getFreeSize(path: string): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute(StatvfsImpl.getFreeSizeSync, path);
      promise.then((ret: NullishType): void => {
        let result = ret as number
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  export function getFreeSize(path: string, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(StatvfsImpl.getFreeSizeSync, path);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }

  export function getTotalSizeSync(path: string): number {
    return StatvfsImpl.getTotalSizeSync(path);
  }

  export function getTotalSize(path: string): Promise<number> {
    return new Promise<number>((resolve: (result: number) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute(StatvfsImpl.getTotalSizeSync, path);
      promise.then((ret: NullishType): void => {
        let result = ret as number
        resolve(result);
      }).catch((e: Error): void => {
        reject(e as BusinessError);
      });
    });
  }

  export function getTotalSize(path: string, callback: AsyncCallback<number>): void {
    let promise = taskpool.execute(StatvfsImpl.getTotalSizeSync, path);
    promise.then((ret: NullishType): void => {
      let e = new BusinessError<void>();
      e.code = 0;
      let result = ret as number;
      callback(e, result);
    }).catch((e: Error): void => {
      callback(e as BusinessError, 0);
    });
  }
}

export default statfs;

class StatvfsImpl {

  static {
    loadLibrary("ani_file_statvfs");
  }

  static native getFreeSizeSync(path: string): number;
  static native getTotalSizeSync(path: string): number;
}
