/*
 * Copyright 2016, akashche at redhat.com
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
 * File:   FileDescriptor.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 11:11 PM
 */

#ifndef UPDATE_CHECKER_FILEDESCRIPTOR_HPP
#define	UPDATE_CHECKER_FILEDESCRIPTOR_HPP

#include <cstdio>

#include "Config.hpp"

namespace checker {

class FileDescriptor {
    std::string filepath;
    mutable FILE* fd;
    uint32_t max_read_bytes;
    size_t bytes_read;
    std::string error;
    
public:
    FileDescriptor(const std::string& filepath, FILE* fd, uint32_t max_read_bytes);
    
    // move emulation
    FileDescriptor(const FileDescriptor& other);
    
    ~FileDescriptor();
    
    FILE* get();
    
    static size_t read_cb(void* buffer, size_t size, void* self);
    
    static int write_cb(const char* buffer, size_t buflen, void* self);
    
    const std::string& get_error();

private:
    FileDescriptor& operator=(const FileDescriptor& other); // = delete
    
    size_t read(void* buffer, size_t size);
        
    size_t write(const char* buffer, size_t buflen);
    
    void append_error(const std::string& err_msg);
};

} // namespace

#endif	/* UPDATE_CHECKER_FILEDESCRIPTOR_HPP */

