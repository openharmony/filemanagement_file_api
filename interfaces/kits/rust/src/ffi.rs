/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

use crate::adapter::{
    create_dir, error_control, get_parent, next_line, read_lines, seek, MakeDirectionMode, SeekPos,
    Str, StringVector,
};
use libc::c_char;
use std::ffi::CString;
use std::ptr::null_mut;

#[no_mangle]
pub unsafe extern "C" fn readLines(path: *const c_char) -> *mut StringVector {
    match read_lines(path) {
        Ok(sv) => sv,
        Err(e) => {
            error_control(e);
            null_mut()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn nextLine(lines: *mut StringVector) -> *mut Str {
    match next_line(lines) {
        Ok(s) => s,
        Err(e) => {
            error_control(e);
            null_mut()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn stringVectorFree(lines: *mut StringVector) {
    if !lines.is_null() {
        let _ = Box::from_raw(lines);
    }
}

#[no_mangle]
pub extern "C" fn lseek(fd: i32, offset: i64, pos: SeekPos) {
    match seek(fd, offset, pos) {
        Ok(_) => {}
        Err(e) => unsafe {
            error_control(e);
        },
    }
}

#[no_mangle]
pub extern "C" fn mkdirs(path: *const c_char, mode: MakeDirectionMode) {
    match create_dir(path, mode) {
        Ok(_) => {}
        Err(e) => unsafe { error_control(e) },
    }
}

#[no_mangle]
pub extern "C" fn getParent(fd: i32) -> *mut Str {
    match get_parent(fd) {
        Ok(str) => str,
        Err(e) => {
            unsafe {
                error_control(e);
            }
            null_mut()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn parentFree(str: *mut Str) {
    if !str.is_null() {
        let string = Box::from_raw(str);
        let _ = CString::from_raw(string.str as *mut c_char);
    }
}
