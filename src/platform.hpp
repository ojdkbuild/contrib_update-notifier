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
 * File:   platform.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:31 PM
 */

#ifndef UPDATE_CHECKER_PLATFORM_HPP
#define	UPDATE_CHECKER_PLATFORM_HPP

#include <cstdio>
#include <string>

#include "Config.hpp"
#include "FileDescriptor.hpp"

/**
 * SHGetKnownFolderPath
 */

namespace checker {
namespace platform {

bool file_exists(const std::string& filepath);

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes);

void close_file(FILE* file);

std::string get_userdata_directory();

void create_directory(const std::string& dirpath);

void delete_file(const std::string& dirpath);

void delete_directory(const std::string& dirpath);

void thread_sleep_millis(uint32_t millis);

std::string current_executable_path();

std::string current_datetime();

} // namespace
} 

#endif	/* UPDATE_CHECKER_PLATFORM_HPP */

