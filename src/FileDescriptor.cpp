/*
 * Copyright 2016 Red Hat, Inc.
 *
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

/* 
 * File:   FileDescriptor.cpp
 * Author: alex
 *
 * Created on October 23, 2016, 8:40 PM
 */

#include "FileDescriptor.hpp"

#include <cerrno>
#include <cstring>

#include "platform.hpp"
#include "utils.hpp"

namespace checker {

FileDescriptor::FileDescriptor(const std::string& filepath, FILE* fd, uint32_t max_read_bytes) :
filepath(filepath.data(), filepath.length()),
fd(fd),
max_read_bytes(max_read_bytes),
bytes_read(0),
error("") {
    if (!this->fd) {
        throw CheckerException("Invalid 'null' FILE specified");
    }
}

// move emulation
FileDescriptor::FileDescriptor(const FileDescriptor& other) :
filepath(other.filepath.data(), other.filepath.length()),
fd(other.fd),
max_read_bytes(other.max_read_bytes),
bytes_read(other.bytes_read),
error(other.error) {
    other.fd = NULL;
}

FileDescriptor::~FileDescriptor() {
    if (fd) {
        platform::close_file(fd);
    }
}

FILE* FileDescriptor::get() {
    return fd;
}

size_t FileDescriptor::read_cb(void* buffer, size_t size, void* self) {
    if (!self) {
        return static_cast<size_t>(-1);
    }
    FileDescriptor* pself = static_cast<FileDescriptor*>(self);
    return pself->read(buffer, size);
}

int FileDescriptor::write_cb(const char* buffer, size_t buflen, void* self) {
    if (!self) {
        return -1;
    }
    FileDescriptor* pself = static_cast<FileDescriptor*> (self);
    size_t res = pself->write(buffer, buflen);
    return static_cast<int>(res);
}

size_t FileDescriptor::read(void* buffer, size_t size) {
    if (!buffer) {
        append_error("'null' buffer specified for 'read', filepath: [" + this->filepath + "]");
        return static_cast<size_t>(-1);
    }
    if (feof(this->fd)) {
        return 0;
    }
    size_t rnum = fread(buffer, 1, size, this->fd);
    if (rnum != size && !feof(this->fd)) {
        append_error(std::string() + "'read' error: [" + strerror(errno) + "]," + 
                " filepath: [" + this->filepath + "]");
        return static_cast<size_t>(-1);
    }
    this->bytes_read += rnum;
    if (this->bytes_read > this->max_read_bytes) {
        append_error("Read limit exceeded: [" + utils::to_string(this->max_read_bytes) + "]," + 
                " filepath: [" + this->filepath + "]");
        return static_cast<size_t>(-1);
    }
    return rnum;
}

size_t FileDescriptor::write(const char* buffer, size_t buflen) {
    if (!buffer) {
        append_error("'null' buffer specified for 'write', filepath: [" + this->filepath + "]");
        return static_cast<size_t>(-1);
    }
    size_t wnum = fwrite(buffer, 1, buflen, this->fd);
    if (wnum != buflen) {
        append_error(std::string() + "'write' error: [" + strerror(errno) + "]," + 
                " filepath: [" + this->filepath + "]");
        return static_cast<size_t>(-1);
    }
    return 0;
}

void FileDescriptor::append_error(const std::string& err_msg) {
    if (err_msg.empty()) return;
    if (!this->error.empty()) {
        this->error.append("\n");
    }
    this->error.append(err_msg);
}

const std::string& FileDescriptor::get_error() {
    return this->error;
}
    

} // namespace
