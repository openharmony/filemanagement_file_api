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
  let muiltDirLocal = srcDirPathLocal+"/test1";
  let srcNoSuffixFileLocal = srcDirPathLocal + '/test';

  let srcDirUriLocal = fileuri.getUriFromPath(srcDirPathLocal);
  let srcFileUriLocal = fileuri.getUriFromPath(srcFilePathLocal);
  let dstDirUriLocal = fileuri.getUriFromPath(dstDirPathLocal);
  let dstFileUriLocal = fileuri.getUriFromPath(dstFilePathLocal);
  let srcNoSuffixFileUriLocal = fileuri.getUriFromPath(srcNoSuffixFileLocal);

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

  function isDirectory(path){
    try {
      console.info("Fs_Copy_Test020:path",path);
      let stat = fs.statSync(path);
      if (stat.isDirectory()) {
        return true;
      }
      return false;
    } catch (error) {
      console.error("Error:", error);
      return false;
    }
  }

  /*
   * @tc.name:Fs_Copy_Test001
   * @tc.desc:test fs.copy with wrong params
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test001", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test001 start.');
    try {
      await fs.copy("aaa", "bbb", (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test002", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test002 start.');
    try {
      await fs.copy(pathDir, "bbb", (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test003", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test003 start.');
    try {
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test004", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test004 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test005", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test005 start.');
    try {
      fs.rmdirSync(srcDirPathLocal);
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test005 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test005 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test005 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    } finally {
      fs.rmdirSync(dstDirPathLocal);
    }
  });

  /*
   * @tc.name:Fs_Copy_Test006
   * @tc.desc:test fs.copy, copy file to file
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test006", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test006 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      let srcFile = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(srcFile.fd, 'ttttttttttttt');
      fs.closeSync(srcFile);
      let dstFile = fs.openSync(dstFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.closeSync(dstFile);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test006 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test006 success. ");
          expect(true).assertTrue();
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test007", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test007 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file.fd);
      await fs.copy(srcFileUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test007 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test007 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test007 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test008
   * @tc.desc:test fs.copy, copy dir to dir
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test008", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test008 start.');
    try {
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.desc:test fs.copy, copy file and write the contents but dest dir isn't exist
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test009", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test009 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test010", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test010 start.');
    try {
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test011", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test011 start.');
    try {
      let flag = false;
      let progressListener = (progress) => {
        flag = true;
        console.info("Fs_Copy_Test011 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test011 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test011 success. ");
          expect(true).assertTrue();
        }
        done();
      })
      setTimeout(function () {
        if (!flag) {
          console.info(TAG, "Fs_Copy_Test013 progressListener error. ");
          expect().assertFail();
          done();
          return;
        }
        expect(true).assertTrue();
        done();
      }, 100);
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test012", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test012 start.');
    try {
      let flag = false;
      let progressListener = (progress) => {
        flag = true;
        console.info("Fs_Copy_Test012 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test012 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test012 success. ");
        }
      })
      setTimeout(function () {
        if (!flag) {
          console.info(TAG, "Fs_Copy_Test013 progressListener error. ");
          expect().assertFail();
          done();
          return;
        }
        expect(true).assertTrue();
        done();
      }, 100);
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test013", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test013 start.');
    try {
      let flag = false;
      let progressListener = (progress) => {
        flag = true;
        console.info("Fs_Copy_Test013 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, options).then((err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test013 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test013 success. ");
        }
      })
      setTimeout(function () {
        if (!flag) {
          console.info(TAG, "Fs_Copy_Test013 progressListener error. ");
          expect().assertFail();
          done();
          return;
        }
        expect(true).assertTrue();
        done();
      }, 100);
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test014", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test014 start.');
    try {
      let flag = false;
      let progressListener = (progress) => {
        flag = true;
        console.info("Fs_Copy_Test014 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, options).then((err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test014 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
          done();
        } else {
          console.info(TAG, "Fs_Copy_Test014 success. ");
        }
        setTimeout(function () {
          if (!flag) {
            console.info(TAG, "Fs_Copy_Test013 progressListener error. ");
            expect().assertFail();
            done();
            return;
          }
          expect(true).assertTrue();
          done();
        }, 100);
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test015", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test015 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test015_first failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
          done();
        } else {
          console.info(TAG, "Fs_Copy_Test015_first success. ");
        }
      })

      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test016", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test016 start.');
    try {
      if (fs.accessSync(srcFilePathLocal)) {
        fs.unlinkSync(srcFilePathLocal);
      }
      fs.mkdirSync(srcDirPathLocal);
      fs.mkdirSync(dstDirPathLocal);
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.closeSync(file);
      await fs.copy(srcFileUriLocal, dstFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test016 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test016 success. ");
          expect(true).assertTrue();
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
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test017", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test017 start.');
    try {
      if (fs.accessSync(srcFilePathLocal)) {
        fs.unlinkSync(srcFilePathLocal);
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, (err) => {
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
   * @tc.desc:test fs.copy, copy empty dir
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test018", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test017 start.');
    try {
      let flag = false;
      let progressListener = (progress) => {
        flag = true;
        console.info("Fs_Copy_Test018 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize);
      };
      let options = {
        "progressListener": progressListener
      }
      if (fs.accessSync(srcFilePathLocal)) {
        fs.unlinkSync(srcFilePathLocal);
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test018 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
          done();
        } else {
          console.info(TAG, "Fs_Copy_Test018 success. ");
        }
      })
      setTimeout(function () {
        if (!flag) {
          console.info(TAG, "Fs_Copy_Test013 progressListener error. ");
          expect().assertFail();
          done();
          return;
        }
        expect(true).assertTrue();
        done();
      }, 100);
    } catch (err) {
      console.error("Fs_Copy_Test018 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test019
   * @tc.desc:test fs.copy file to itself
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test019", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test019 start.');
    try {
      await fs.copy(srcFileUriLocal, srcFileUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test019 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test019 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test019 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect(false).assertTrue();
      done();
    }
  });

  /*
    * @tc.name:Fs_Copy_Test020
    * @tc.desc:test fs.copy, copy muilt empty dir
    * @tc.type: FUNC
    * @tc.require: #I8UV2F
    */
  it("Fs_Copy_Test020", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test020 start.');
    try {
      fs.mkdirSync(srcDirPathLocal+"/test1");
      fs.mkdirSync(srcDirPathLocal+"/test2");
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test020 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize);
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test020 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          if (isDirectory(dstDirPathLocal + "/src/test1") && isDirectory(dstDirPathLocal + "/src/test2")) {
            console.info(TAG, "Fs_Copy_Test020 success. ");
            expect(true).assertTrue();
            done();
            return;
          }
          expect().assertFail();
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
  * @tc.desc:test fs.copy, copy muilt dir contain files
  * @tc.type: FUNC
  * @tc.require: #I8UV2F
  */
  it("Fs_Copy_Test021", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test021 start.');
    try {
      let file1 = fs.openSync(muiltDirLocal + "/test1.txt", fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file1.fd, "ttttttttttttt");
      fs.closeSync(file1);
      let file2 = fs.openSync(muiltDirLocal + "/test2.txt", fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file2.fd, "ttttttttttttt");
      fs.closeSync(file2);
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test021 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcDirUriLocal, dstDirUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test021 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          if (fs.accessSync(dstDirPathLocal +"/src/test1/test1.txt") && fs.accessSync(dstDirPathLocal + "/src/test1/test2.txt")) {
            console.info(TAG, "Fs_Copy_Test021 success. ");
            expect(true).assertTrue();
            done();
            return;
          }
          expect().assertFail();
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
   * @tc.name:Fs_Copy_Test022
   * @tc.desc:test fs.copy, copy file 10 times
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test022", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test022 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      // execute the copy operation 10 times
      for (let i = 0; i < 10; i++) {
        const dstStitchFilePath = `${dstDirUriLocal}/file_${i}.txt`;
        await fs.copy(srcFileUriLocal, dstStitchFilePath, (err) => {
          if (err) {
            console.info(TAG, "Fs_Copy_Test022 failed, with error message: " + err.message + ", error code: " + err.code);
            expect().assertFail();
            done();
          } else {
            console.info(TAG, "Fs_Copy_Test022 success. ");
          }
        })
      }
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error("Fs_Copy_Test022 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test023
   * @tc.desc:test fs.copy, copy 1KB file
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test023", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test023 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      let content = 't'.repeat(1024);
      fs.writeSync(file.fd, content);
      fs.closeSync(file);
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test023 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test023 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test023 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test023 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test024
   * @tc.desc:test fs.copy, copy 100MB file
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test024", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test024 start.');
    try {
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      let content = 't'.repeat(1024 * 1024 * 100);
      fs.writeSync(file.fd, content);
      fs.closeSync(file);
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test024 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      await fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test024 failed, with error message: " + err.message + ", error code: " + err.code);
          expect().assertFail();
        } else {
          console.info(TAG, "Fs_Copy_Test024 success. ");
          expect(true).assertTrue();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test024 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test025
   * @tc.desc:test fs.copy dir to own directory1
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test025", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test025 start.');
    try {
      await fs.copy(srcDirUriLocal, srcDirUriLocal+"/", (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test025 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test025 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test025 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test025
   * @tc.desc:test fs.copy dir to own directory2
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test026", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test026 start.');
    try {
      await fs.copy(srcDirUriLocal+"/", srcDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test026 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test026 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test026 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test027
   * @tc.desc:test fs.copy file wiht no suffix
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test027", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test027 start.');
    try {
      let file = fs.openSync(srcNoSuffixFileLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, "ttttttttttttt");
      fs.closeSync(file);
      await fs.copy(srcNoSuffixFileUriLocal, dstDirUriLocal, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test027 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test027 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test027 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });
  /*
   * @tc.name:Fs_Copy_Test028
   * @tc.desc:test fs.copy file with indiscriminate parameter
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test028", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test028 start.');
    try {
      let srcPath = pathDir+"/./pppppp";
      let destPath =pathDir+ "/ttt/pppppp";
      let srcUriPath = fileuri.getUriFromPath(srcPath);
      let destUriPath = fileuri.getUriFromPath(destPath);
      fs.mkdirSync(srcPath);
      await fs.copy(srcUriPath, destUriPath, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test028 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test028 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test028 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test029
   * @tc.desc:test fs.copy file with indiscriminate parameter2
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test029", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test029 start.');
    try {
      let srcPath = pathDir+"/.////ssssss///";
      let destPath =pathDir+ "/ttt/ssssss//////";
      let srcUriPath = fileuri.getUriFromPath(srcPath);
      let destUriPath = fileuri.getUriFromPath(destPath);
      fs.mkdirSync(srcPath);
      await fs.copy(srcUriPath, destUriPath, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test029 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test029 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test029 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
  * @tc.name:Fs_Copy_Test030
  * @tc.desc:test fs.copy dir to subdir
  * @tc.type: FUNC
  * @tc.require: #I8UV2F
  */
  it("Fs_Copy_Test030", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test030 start.');
    try {
      let srcPath = pathDir+"/test1/";
      let destPath =pathDir+ "/test1/testttt2";
      let srcUriPath = fileuri.getUriFromPath(srcPath);
      let destUriPath = fileuri.getUriFromPath(destPath);
      fs.mkdirSync(srcPath);
      await fs.copy(srcUriPath, destUriPath, (err) => {
        if (err) {
          console.info(TAG, "Fs_Copy_Test030 failed, with error message: " + err.message + ", error code: " + err.code);
          expect(true).assertTrue();
        } else {
          console.info(TAG, "Fs_Copy_Test030 success. ");
          expect().assertFail();
        }
        done();
      })
    } catch (err) {
      console.error("Fs_Copy_Test030 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:Fs_Copy_Test031
   * @tc.desc:test fs.copy, same task
   * @tc.type: FUNC
   * @tc.require: #I8UV2F
   */
  it("Fs_Copy_Test031", 0, async function (done) {
    console.info(TAG, 'Fs_Copy_Test031 start.');
    try {
      let flag1 = false;
      let flag2 = false;
      let progressListener = (progress) => {
        console.info("Fs_Copy_Test031 progressListener in, progressSize: " + progress.processedSize + ", totalSize: " + progress.totalSize +
            " progress： " + (progress.processedSize / progress.totalSize * 100).toFixed(2) + "%");
      };
      let options = {
        "progressListener": progressListener
      }
      if (fs.accessSync(srcFilePathLocal)) {
        fs.unlinkSync(srcFilePathLocal);
      }
      let file = fs.openSync(srcFilePathLocal, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
      fs.writeSync(file.fd, 'ttttttttttttt');
      fs.closeSync(file);
      const task1 = new Promise(resolve => {
        setTimeout(() => {
          fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
            if (err) {
              console.info(TAG, "Fs_Copy_Test031_first failed, with error message: " + err.message + ", error code: " + err.code);
              expect().assertFail();
            } else {
              flag1 = true;
              console.info(TAG, "Fs_Copy_Test031_first success. ");
            }
          })
          resolve();
        }, 10);
      });

      const task2 = new Promise(resolve => {
        setTimeout(() => {
          try {
            fs.copy(srcFileUriLocal, dstFileUriLocal, options, (err) => {
              flag2 = true;
              if (err) {
                console.info(TAG, "Fs_Copy_Test031_second failed, with error message: " + err.message + ", error code: " + err.code);
              } else {
                console.info(TAG, "Fs_Copy_Test031_second success. ");;
              }
              expect().assertFail();
            })
            resolve();
          } catch (err) {
            console.error("Fs_Copy_Test031_second failed with invalid param: " + err.message + ", error code: " + err.code);
          }
        }, 20);
      });
      //the same task isn't execute
      Promise.all([task1, task2]).then(() => {
        if (flag1 && !flag2) {
          expect(true).assertTrue();
          done();
        }
      });
      done();
    } catch (err) {
      console.error("Fs_Copy_Test031 failed with invalid param: " + err.message + ", error code: " + err.code);
      expect().assertFail();
      done();
    } finally {
      const timerId = setTimeout(() => {
        fs.rmdirSync(srcDirPathLocal);
        fs.rmdirSync(dstDirPathLocal);
        clearTimeout(timerId);
      }, 1000);
    }
  });
});