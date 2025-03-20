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

type DataLevel = 's0' | 's1' | 's2' | 's3' | 's4';

function setSecurityLabel(path: string, type: DataLevel): Promise<void> {
  return new Promise<void>((resolve: (result: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
    let promise = taskpool.execute((path: string, type: DataLevel): void => SecurityLabelImpl.setSecurityLabelSync(path, type), path, type);
    promise.then((ret: NullishType): void => {
      resolve(undefined);
    }).catch((e: BusinessError): void => {
      reject(e);
    });
  });
}

function setSecurityLabel(path: string, type: DataLevel, callback: AsyncCallback<void>): void {
  let promise = taskpool.execute((path: string, type: DataLevel): void => SecurityLabelImpl.setSecurityLabelSync(path, type), path, type);
  promise.then((ret: NullishType): void => {
    let e = new BusinessError<void>();
    e.code = 0;
    callback(e, undefined);
  }).catch((e: BusinessError): void => {
    callback(e, undefined);
  });
}

function setSecurityLabelSync(path: string, type: DataLevel): void {
  return SecurityLabelImpl.setSecurityLabelSync(path, type);
}

class SecurityLabelImpl {

  static {
    loadLibrary("ani_securitylabel_class");
  }

  static native setSecurityLabelSync(path: string, type: DataLevel): void;
}
