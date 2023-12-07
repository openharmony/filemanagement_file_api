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
    create_dir, error_control, get_parent, next_line, reader_iterator, seek, cut_file_name,
    MakeDirectionMode, SeekPos, Str,
};
use libc::{c_char, c_int, c_longlong, c_void};
use std::ffi::CString;
use std::ptr::null_mut;

#[no_mangle]
pub unsafe extern "C" fn ReaderIterator(path: *const c_char) -> *mut c_void {
    match reader_iterator(path) {
        Ok(sv) => sv,
        Err(e) => {
            error_control(e);
            null_mut()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn NextLine(iter: *mut c_void) -> *mut Str {
    match next_line(iter) {
        Ok(s) => s,
        Err(e) => {
            error_control(e);
            null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn Lseek(fd: i32, offset: i64, pos: SeekPos) -> c_longlong {
    match seek(fd, offset, pos) {
        Ok(pos) => pos as c_longlong,
        Err(e) => unsafe {
            error_control(e);
            -1
        },
    }
}

#[no_mangle]
pub extern "C" fn Mkdirs(path: *const c_char, mode: MakeDirectionMode) -> c_int {
    match create_dir(path, mode) {
        Ok(_) => 0,
        Err(e) => unsafe {
            error_control(e);
            -1
        },
    }
}

#[no_mangle]
pub extern "C" fn GetParent(fd: i32) -> *mut Str {
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
pub unsafe extern "C" fn StrFree(str: *mut Str) {
    if !str.is_null() {
        let string = Box::from_raw(str);
        let _ = CString::from_raw(string.str);
    }
}

#[no_mangle]
pub unsafe extern "C" fn CutFileName(path: *const c_char, size: usize) -> *mut Str {
    cut_file_name(path, size)
}
