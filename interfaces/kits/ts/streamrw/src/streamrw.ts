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
const fileIo = requireNapi('file.fs');

interface ReadStreamOptions {
    start?: number;
    end?: number;
}

interface WriteStreamOptions {
    start?: number;
    mode?: number;
}

class ReadStream extends stream.Readable {
    private pathInner: string;
    private bytesReadInner: number;
    private offset: number;
    private start?: number;
    private end?: number;
    // @ts-ignore
    private stream?: fileIo.Stream;

    constructor(path: string, options?: ReadStreamOptions) {
        super();
        this.pathInner = path;
        this.bytesReadInner = 0;
        this.start = options?.start;
        this.end = options?.end;
        this.stream = fileIo.createStreamSync(this.pathInner, 'r');
        this.offset = this.start ?? 0;
    }

    get path(): string {
        return this.pathInner;
    }

    get bytesRead(): number {
        return this.bytesReadInner;
    }

    // @ts-ignore
    seek(offset: number, whence?: fileIo.WhenceType): number {
        if (whence === undefined) {
            this.offset = this.stream?.seek(offset);
        } else {
            this.offset = this.stream?.seek(offset, whence);
        }
        return this.offset;
    }

    close(): void {
        this.stream?.close();
    }

    doInitialize(callback: Function): void {
        callback();
    }

    doRead(size: number): void {
        let readSize = size;
        if (this.end !== undefined) {
            if (this.offset > this.end) {
                this.push(null);
                return;
            }
            if (this.offset + readSize > this.end) {
                readSize = this.end - this.offset;
            }
        }
        let buffer = new ArrayBuffer(readSize);
        const off = this.offset;
        this.offset += readSize;
        this.stream?.read(buffer, { offset: off, length: readSize })
            .then((readOut: number) => {
                if (readOut > 0) {
                    this.bytesReadInner += readOut;
                    this.push(new Uint8Array(buffer.slice(0, readOut)));
                }
                if (readOut !== readSize || readOut < size) {
                    this.offset = this.offset - readSize + readOut;
                    this.push(null);
                }
            });
    }
}

class WriteStream extends stream.Writable {
    private pathInner: string;
    private bytesWrittenInner: number;
    private offset: number;
    private mode: string;
    private start?: number;
    // @ts-ignore
    private stream?: fileIo.Stream;

    constructor(path: string, options?: WriteStreamOptions) {
        super();
        this.pathInner = path;
        this.bytesWrittenInner = 0;
        this.start = options?.start;
        this.mode = this.convertOpenMode(options?.mode);
        this.stream = fileIo.createStreamSync(this.pathInner, this.mode);
        this.offset = this.start ?? 0;
    }

    get path(): string {
        return this.pathInner;
    }

    get bytesWritten(): number {
        return this.bytesWrittenInner;
    }

    // @ts-ignore
    seek(offset: number, whence?: fileIo.WhenceType): number {
        if (whence === undefined) {
            this.offset = this.stream?.seek(offset);
        } else {
            this.offset = this.stream?.seek(offset, whence);
        }
        return this.offset;
    }

    close(): void {
        this.stream?.close();
    }

    doInitialize(callback: Function): void {
        callback();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        this.stream?.write(chunk, { offset: this.offset })
            .then((writeIn: number) => {
                this.offset += writeIn;
                this.bytesWrittenInner += writeIn;
                callback();
            })
            .finally(() => {
                this.stream?.flush();
            });
    }

    convertOpenMode(mode?: number): string {
        let modeStr = 'w';
        if (mode === undefined) {
            return modeStr;
        }
        if (mode & fileIo.OpenMode.WRITE_ONLY) {
            modeStr = 'w';
        }
        if (mode & fileIo.OpenMode.READ_WRITE) {
            modeStr = 'w+';
        }
        if ((mode & fileIo.OpenMode.WRITE_ONLY) && (mode & fileIo.OpenMode.APPEND)) {
            modeStr = 'a';
        }
        if ((mode & fileIo.OpenMode.READ_WRITE) && (mode & fileIo.OpenMode.APPEND)) {
            modeStr = 'a+';
        }
        return modeStr;
    }
}

export default {
    ReadStream: ReadStream,
    WriteStream: WriteStream,
};
