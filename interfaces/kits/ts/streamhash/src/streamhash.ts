/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

declare function requireNapi(napiModuleName: string): any;
const stream = requireNapi('util.stream');
const hash = requireNapi('file.hash');

class HashStream extends stream.Transform {
    // @ts-ignore
    hs: hash.HashStream;
    hashBuf?: ArrayBuffer;

    constructor(algorithm: string) {
        super();
        this.hs = new hash.HashStream(algorithm);
    }

    digest(): string {
        return this.hs.digest();
    }

    update(data: ArrayBuffer) {
        this.hs.update(data);
    }

    doTransform(chunk: string, encoding: string, callback: Function) {
        const buf = new Uint8Array(chunk.split('').map(x => x.charCodeAt(0))).buffer;
        this.hs.update(buf);
        this.push(chunk);
        callback();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback();
    }

    doFlush(callback: Function) {
        callback();
    }
}

export default {
    HashStream: HashStream,
}
