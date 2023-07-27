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

use libc::{__errno_location, c_char, c_uint};
use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::{BufRead, BufReader, Error, ErrorKind, Seek, SeekFrom};
#[cfg(unix)]
use std::os::unix::io::{FromRawFd, RawFd};
use std::path::PathBuf;
use std::ptr::null_mut;
use std::{fs, mem};

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
    pub str: *const c_char,
    /// The length of string.
    pub len: c_uint,
}

/// Structure for storing file information by line.
#[derive(Debug, Default)]
pub struct StringVector {
    pub(crate) vec: Vec<String>,
    pub(crate) len: usize,
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
            _ => unreachable!("Unexpected error type"),
        }
    }
}

pub(crate) unsafe fn read_lines(path: *const c_char) -> Result<*mut StringVector, Error> {
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
    let mut lines = StringVector::default();
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);
    loop {
        let mut line = String::new();
        let len = reader.read_line(&mut line)?;
        if len > 0 {
            lines.vec.push(line);
        } else {
            return Ok(Box::into_raw(Box::new(lines)));
        }
    }
}

pub(crate) unsafe fn next_line(lines: *mut StringVector) -> Result<*mut Str, Error> {
    if lines.is_null() {
        return Err(Error::new(ErrorKind::InvalidInput, "Invalid input"));
    }
    let lines = &mut *lines;
    let cnt = lines.len;
    if cnt < lines.vec.len() {
        let line = lines.vec[cnt].as_ptr() as *const c_char;
        let len = lines.vec[cnt].len() as c_uint;
        let item = Str { str: line, len };
        lines.len += 1;
        Ok(Box::into_raw(Box::new(item)))
    } else {
        Ok(null_mut())
    }
}

pub(crate) fn seek(fd: i32, offset: i64, pos: SeekPos) -> Result<(), Error> {
    let mut file = unsafe { File::from_raw_fd(fd as RawFd) };

    match pos {
        SeekPos::Start => file.seek(SeekFrom::Start(offset as u64))?,
        SeekPos::Current => file.seek(SeekFrom::Current(offset))?,
        SeekPos::End => file.seek(SeekFrom::End(offset))?,
    };

    mem::forget(file);
    Ok(())
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
                    str: par_path.into_raw() as *const c_char,
                    len,
                };
                return Ok(Box::into_raw(Box::new(item)));
            }
        }
    }
    Ok(null_mut())
}
