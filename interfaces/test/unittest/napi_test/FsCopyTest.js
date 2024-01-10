/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

import fs from '@ohos.file.fs';
import fileuri from '@ohos.file.fileuri';

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';

const TAG = 'FsCopyTest';
describe("FsCopyTest", function () {
  let pathDir = '/data/storage/el2/base/haps/entry/files';
  let srcDirPathLocal = pathDir + "/src";
  let srcFilePathLocal = srcDirPathLocal + '/srcFile.txt';
  let dstDirPathLocal = pathDir + "/dest";
  let dstFilePathLocal = dstDirPathLocal + '/dstFile.txt';

  let srcDirUriLocal = fileuri.getUriFromPath(srcDirPathLocal);
  let srcFileUriLocal = fileuri.getUriFromPath(srcFilePathLocal);
  let dstDirUriLocal = fileuri.getUriFromPath(dstDirPathLocal);
  let dstFileUriLocal = fileuri.getUriFromPath(dstFilePathLocal);

  beforeAll(function () {
    console.info(TAG, 'beforeAll called')
  });

  afterAll(function () {
    console.info(TAG, 'afterAll called')
  });

  beforeEach(function () {
    console.info(TAG, 'beforeEach called')
  });

  afterEach(function () {
    console.info(TAG, 'afterEach called')
  });

  /*
   * @tc.name:Fs_Copy_Test001
   * @tc.desc:test fs.copy with wrong params
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test001", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test001 start.');
    try {
      fs.copy("aaa", "bbb", (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test001 failed, with error message: " + err.message + ", error code: " + err.code);
        } else {
          console.info(TAG, "Fs_Copy_Test001 success. ");
        }
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test001 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test002
   * @tc.desc:test fs.copy with sandbox path
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test002", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test002 start.');
    try {
      fs.copy(pathDir, "bbb", (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test002 failed, with error message: " + err.message + ", error code: " + err.code);
        } else {
          console.info(TAG, "Fs_Copy_Test002 success. ");
        }
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test002 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test003
   * @tc.desc:test fs.copy with not exist dir
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test003", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test003 start.');
    try {
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.error(TAG, "Fs_Copy_Test003 failed: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test003 success.");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test003 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test004
   * @tc.desc:test fs.copy dir to itself
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test004", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test004 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test004 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test004 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test004 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test005
   * @tc.desc:test fs.copy, copy file, src isn't exist
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test005", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test005 start.');
    try {
      fs.rmdirSync(srcDirPathLocal);
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test005 failed, with error message: " + err.message + ", error code: " + err.code);
        } else {
          console.info(TAG, "Fs_Copy_Test005 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test005 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    } finally {
      fs.rmdirSync(dstDirPathLocal);
    }
  });

  /*
   * @tc.name:Fs_Copy_Test006
   * @tc.desc:test fs.copy, copy file to file
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test006", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test006 start.');
    try {
      console.info("Fs_Copy_Test006 111111111111111111 ");
      fs.mkdirSync(srcDirPathLocal);          //SRC
      console.info("Fs_Copy_Test006 222222222222222222 ");
      fs.mkdirSync(dstDirPathLocal);
      console.info("Fs_Copy_Test006 3333333333333333333 ");
      let srcFile = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(srcFile.fd, 'ttttttttttttt');
      fs.closeSync(srcFile);
      let dstFile = fs.openSync(dstFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.closeSync(dstFile);
      fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test006 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          if (dstFile.totalSize > 0) {
            console.info(TAG, "Fs_Copy_Test006 success. ");
            expect(true).assertTrue();
          }
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test006 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test007
   * @tc.desc:test fs.copy, copy file to dir
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test007", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test007 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file.fd);
      fs.copy(srcFileUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test007 failed, with error message: " + err.message + ", error code: " + err.code);
        } else {
          console.info(TAG, "Fs_Copy_Test007 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test007 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test008
   * @tc.desc:test fs.copy, copy dir to dir
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test008", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test008 start.');
    try {
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test008 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test008 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test008 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    } finally {
      fs.rmdirSync(srcDirPathLocal);
      fs.rmdirSync(dstDirPathLocal);
    }
  });

  /*
    * @tc.name:Fs_Copy_Test009
    * @tc.desc:test fs.copy, copy file and write the contents but dest isn't exist
    * @tc.type: FUNC
    * @tc.require: issueNumber
    */
  it("Fs_Copy_Test009", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test009 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test009 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test009 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test009 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    } finally {
      fs.rmdirSync(srcDirPathLocal);
    }
  });

  /*
    * @tc.name:Fs_Copy_Test010
    * @tc.desc:test fs.copy, copy dir and file in the src path 
    * @tc.type: FUNC
    * @tc.require: issueNumber
    */
  it("Fs_Copy_Test010", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test010 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test010 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test010 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test010 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test011
   * @tc.desc:test fs.copy, copy file to file with listener
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test011", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test011 start.');
    try {
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test011 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test011 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test011 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test011 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test012
   * @tc.desc:test fs.copy, copy dir to dir with listener
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test012", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test012 start.');
    try {
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test012 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      fs.copy(srcDirUriLocal, dstDirUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test012 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test012 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test012 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test013
   * @tc.desc:test fs.copy, copy file to file with listener promise
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test013", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test013 start.');
    try {
      console.info("Fs_Copy_Test013 11111111111111111111");
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test013 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      fs.copy(srcFileUriLocal, dstFileUriLocal, options).then((err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test013 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test013 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test013 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test014
   * @tc.desc:test fs.copy, copy dir to dir with listener promise
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test014", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test014 start.');
    try {
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test014 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      fs.copy(srcDirUriLocal, dstDirUriLocal, options).then((err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test014 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test014 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test014 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test015
   * @tc.desc:test fs.copy, copy file 2 times
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test015", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test015 start.');
    let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
    fs.writeSync(file.fd, 'ttttttttttttt');
    fs.closeSync(file);
    try {
      fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test015_first failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
          done();
        } else {
          console.info(TAG, "Fs_Copy_Test015_first success. ");
        }
      })

      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test015_second failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test015_second success. ");
          expect(true).assertTrue();
          fs.rmdirSync(srcDirPathLocal);
          fs.rmdirSync(dstDirPathLocal);
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test015 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test016
   * @tc.desc:test fs.copy, copy empty file
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test016", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test016 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.closeSync(file);
      fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test016 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test016 success. ");
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test016 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test017
   * @tc.desc:test fs.copy, copy empty dir
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test017", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test017 start.');
    try {
      fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test017 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test017 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test017 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test018
   * @tc.desc:test fs.copy file to itself
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test018", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test018 start.');
    try {
      // fs.mkdirSync(srcDirPathLocal);
      fs.copy(srcFileUriLocal, srcFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test018 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test018 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test018 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test020
   * @tc.desc:test fs.copy, copy file 10 times
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test019", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test019 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      // 执行拷贝操作10次
      for (let i = 0; i < 10; i++) {
        const dstStitchFilePath = `${dstDirUriLocal}/file_${i}.txt`;
        fs.copy(srcFileUriLocal, dstStitchFilePath, (err) => {
          if (err) {
            console.info(TAG, "Fs_Copy_Test019 failed, with error message: " + err.message + ", error code: " + err.code);
            expect().assertFail();
            done();
          } else {
            console.info(TAG, "Fs_Copy_Test019 success. ");
          }
        })
      }
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error("Fs_Copy_Test019 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
 * @tc.name:Fs_Copy_Test020
 * @tc.desc:test fs.copy, copy 1KB file
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
  it("Fs_Copy_Test020", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test020 start.');
    try {
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test020 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      let srcFile = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      let content = 't'.repeat(1024);
      fs.writeSync(srcFile.fd, content);
      fs.closeSync(srcFile);
      let dstFile = fs.openSync(dstFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.closeSync(dstFile);
      fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test020 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          if (dstFile.totalSize > 0) {
            console.info(TAG, "Fs_Copy_Test020 success. ");
            expect(true).assertTrue();
          }
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test020 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test021
   * @tc.desc:test fs.copy, copy 1000MB file
   * @tc.type: FUNC
   * @tc.require: issueNumber
   */
  it("Fs_Copy_Test021", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test021 start.');
    try {
      console.info("Fs_Copy_Test021 111111111111111 ");
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      let content = 't'.repeat(1024 * 1024 * 100);
      fs.writeSync(file.fd, content);
      fs.closeSync(file);
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test021 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test021 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test021 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test021 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test020
   * @tc.desc:test fs.copy, same task
   * @tc.type: FUNC
   * @tc.require: issueNumber
    */
  it("Fs_Copy_Test022", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test022 start.');
    try {
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test022 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
          " 百分比： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      if(fs.accessSync(srcFilePathLocal)){
        fs.unlinkSync(srcFilePathLocal);
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test022_first failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test022_first success. ");
        }
        done();
      })
      //the same task isn't execute
      fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test022_second failed, with error message: " + err.message + ", error code: " + err.code);
        } else {
          console.info(TAG, "Fs_Copy_Test022_second success. ");;
        }
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test022 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(true).assertTrue();
      done();
    } finally {
      const timerId = setTimeout(() => {
        fs.rmdirSync(srcDirPathLocal);
        fs.rmdirSync(dstDirPathLocal);
        clearTimeout(timerId);
      }, 100);
    }
  });
});