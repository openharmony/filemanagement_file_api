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

use hilog_rust::{error, hilog, HiLogLabel, LogType};
use libc::{__errno_location, c_char, c_uint, c_void};
use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::{BufRead, BufReader, Error, ErrorKind, Seek, SeekFrom};
#[cfg(unix)]
use std::os::unix::io::{FromRawFd, RawFd};
use std::path::PathBuf;
use std::ptr::null_mut;
use std::{fs, mem};

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD004388,
    tag: "file_api",
};

/// Enumeration of `lseek` interface to seek within a file.
#[repr(C)]
#[allow(dead_code)]
pub enum SeekPos {
    Start,
    Current,
    End,
}

/// Enumeration of `mkdirs` interface to choose ways to create the direction.
#[repr(C)]
#[allow(dead_code)]
pub enum MakeDirectionMode {
    Single,
    Multiple,
}

/// Structure for storing string and its effective length.
#[repr(C)]
pub struct Str {
    /// C string.
    pub str: *mut c_char,
    /// The length of string.
    pub len: c_uint,
}

pub(crate) unsafe fn error_control(err: Error) {
    let errno_pos = __errno_location();
    if let Some(raw) = err.raw_os_error() {
        *errno_pos = raw;
    } else {
        match err.kind() {
            ErrorKind::NotFound => *errno_pos = 2,
            ErrorKind::PermissionDenied => *errno_pos = 13,
            ErrorKind::AlreadyExists => *errno_pos = 17,
            ErrorKind::InvalidInput => *errno_pos = 22,
            ErrorKind::InvalidData => *errno_pos = 61,
            _ => {
                *errno_pos = 13900042;
                error!(LOG_LABEL, "Unknown error is : {}", @public(err));
            }
        }
    }
}

pub(crate) unsafe fn reader_iterator(path: *const c_char) -> Result<*mut c_void, Error> {
    if path.is_null() {
        return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
    }
    let path = CStr::from_ptr(path);
    let path = match path.to_str() {
        Ok(p) => p,
        Err(_) => {
            return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
        }
    };
    let file = File::open(path)?;
    let reader = BufReader::new(file);
    Ok(Box::into_raw(Box::new(reader)) as *mut c_void)
}

pub(crate) unsafe fn next_line(iter: *mut c_void) -> Result<*mut Str, Error> {
    if iter.is_null() {
        return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
    }
    let reader = &mut *(iter as *mut BufReader<File>);
    let mut line = String::new();
    let len = reader.read_line(&mut line)? as c_uint;
    if len > 0 {
        let line_bytes = line.into_bytes();
        let line = CString::from_vec_unchecked(line_bytes);
        let item = Str {
            str: line.into_raw(),
            len,
        };
        Ok(Box::into_raw(Box::new(item)))
    } else {
        Ok(null_mut())
    }
}

pub(crate) fn seek(fd: i32, offset: i64, pos: SeekPos) -> Result<u64, Error> {
    let mut file = unsafe { File::from_raw_fd(fd as RawFd) };

    let new_pos = match pos {
        SeekPos::Start => file.seek(SeekFrom::Start(offset as u64)),
        SeekPos::Current => file.seek(SeekFrom::Current(offset)),
        SeekPos::End => file.seek(SeekFrom::End(offset)),
    };

    mem::forget(file);
    new_pos
}

pub(crate) fn create_dir(path: *const c_char, mode: MakeDirectionMode) -> Result<(), Error> {
    if path.is_null() {
        return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
    }
    let path = unsafe { CStr::from_ptr(path) };
    let path = match path.to_str() {
        Ok(p) => p,
        Err(_) => {
            return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
        }
    };
    match mode {
        MakeDirectionMode::Single => fs::create_dir(path),
        MakeDirectionMode::Multiple => fs::create_dir_all(path),
    }
}

pub(crate) fn get_parent(fd: i32) -> Result<*mut Str, Error> {
    let mut p = PathBuf::from("/proc/self/fd");
    p.push(&fd.to_string());
    let path = fs::read_link(&p)?;
    match path.as_path().parent() {
        None => {}
        Some(parent) => {
            if let Some(str) = parent.to_str() {
                // When the return value of `Path::parent()` is `Some(s)`, `s` will not be empty
                // string.
                let par_path = CString::new(str).unwrap();
                let len = par_path.as_bytes().len() as c_uint;
                let item = Str {
                    str: par_path.into_raw(),
                    len,
                };
                return Ok(Box::into_raw(Box::new(item)));
            }
        }
    }
    Ok(null_mut())
}

pub(crate) unsafe fn cut_file_name(path: *const c_char, size: usize) -> *mut Str {
    let path_str = match CStr::from_ptr(path).to_str() {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };
    let len = path_str.chars().count();
    let size = size.min(len);

    let mut sliced_str = String::from(path_str);
    for _ in 0..size {
        let _ = sliced_str.pop();
    }

    let result = match CString::new(sliced_str.clone()) {
        Ok(s) => Str {
            str: s.into_raw(),
            len: sliced_str.as_bytes().len() as c_uint,
        },
        Err(_) => return std::ptr::null_mut(),
    };
    Box::into_raw(Box::new(result))
}
