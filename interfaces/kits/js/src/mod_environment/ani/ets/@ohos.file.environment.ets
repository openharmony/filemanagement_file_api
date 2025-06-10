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

namespace Environment {
  export function getStorageDataDir(): Promise<string> {
    return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
      let promise = taskpool.execute(EnvironmentImpl.getStorageDataDirSync);
        promise.then((ret: NullishType): void => {
            let result = ret as string;
            resolve(result);
        }).catch((e: Error): void => {
            reject(e as BusinessError);
        });
        });
  }

  export function getStorageDataDir(callback: AsyncCallback<string>): void {
    let promise = taskpool.execute(EnvironmentImpl.getStorageDataDirSync);
      promise.then((ret: NullishType): void => {
        let e = new BusinessError<void>();
        e.code = 0;
        let result = ret as string;
        callback(e, result);
      }).catch((e: Error): void => {
        callback(e as BusinessError, "");
      });
  }

  export function getUserDataDir(): Promise<string> {
    return new Promise<string>((resolve: (result: string) => void, reject: (e: BusinessError<void>) => void): void => {
        let promise = taskpool.execute(EnvironmentImpl.getUserDataDirSync);
        promise.then((ret: NullishType): void => {
            let result = ret as string;
            resolve(result);
        }).catch((e: Error): void => {
            reject(e as BusinessError);
        });
        });
  }
  export function getUserDataDir(callback: AsyncCallback<string>): void {
    let promise = taskpool.execute(EnvironmentImpl.getUserDataDirSync);
      promise.then((ret: NullishType): void => {
        let e = new BusinessError<void>();
        e.code = 0;
        let result = ret as string;
        callback(e, result);
      }).catch((e: Error): void => {
        callback(e as BusinessError, "");
      });
  }

  export function getUserDownloadDir(): string {
    return EnvironmentImpl.getUserDownloadDirSync();
  }

  export function getUserDesktopDir(): string {
    return EnvironmentImpl.getUserDesktopDirSync();
  }

  export function getUserDocumentDir(): string {
    return EnvironmentImpl.getUserDocumentDirSync();
  }

  export function getExternalStorageDir(): string {
    return EnvironmentImpl.getExternalStorageDirSync();
  }

  export function getUserHomeDir(): string {
    return EnvironmentImpl.getUserHomeDirSync();
  }

}

export default Environment;

class EnvironmentImpl {

  static {
    loadLibrary("ani_file_environment");
  }

  static native getStorageDataDirSync(): string;
  static native getUserDataDirSync(): string;
  static native getUserDownloadDirSync(): string;
  static native getUserDesktopDirSync(): string;
  static native getUserDocumentDirSync(): string;
  static native getExternalStorageDirSync(): string;
  static native getUserHomeDirSync(): string;
}
