#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import shutil
import argparse
import subprocess
import sys

def run_command(in_cmd):
    print(" ".join(in_cmd))
    proc = subprocess.Popen(in_cmd, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          universal_newlines=True,
                          shell=False)
    stdout, stderr = proc.communicate()
    if stdout != "":
        raise Exception(stdout)

if __name__ == '__main__':
    PARSER_INST = argparse.ArgumentParser()
    PARSER_INST.add_argument('--dst-file',
                        help='the converted target file')
    PARSER_INST.add_argument('--module-path',
                        help='the module path')
    PARSER_INST.add_argument('--out-file',
                        help='js output file')
    PARSER_INST.add_argument('--out-filePath',
                        help='js output filePath')
    PARSER_INST.add_argument('--relative-path',
                        help='the code root path relative the root_build_dir')
    INPUT_ARGUMENTS = PARSER_INST.parse_args()

    BUILD_PATH = os.path.abspath(os.path.join(os.getcwd(), INPUT_ARGUMENTS.relative_path))
    os.chdir(("%s" + INPUT_ARGUMENTS.module_path) % BUILD_PATH)
    os_name = sys.platform
    NODE_PATH = '../../../../../../../prebuilts/build-tools/common/nodejs/current/bin/node'
    if not os.path.exists(NODE_PATH):
        raise Exception('error:NO such file or directory')
    TSC_PATH = '../../../../../../../third_party/typescript/bin/tsc'
    CMD_INST = [NODE_PATH, TSC_PATH, "--outDir", INPUT_ARGUMENTS.out_filePath]
    run_command(CMD_INST)
    if not os.path.exists(INPUT_ARGUMENTS.out_file):
        raise Exception('error:NO such file or directory')
    CMD_INST = shutil.copy(INPUT_ARGUMENTS.out_file, INPUT_ARGUMENTS.dst_file)

    CMD_INST = shutil.rmtree(INPUT_ARGUMENTS.out_filePath)

    exit(0)
