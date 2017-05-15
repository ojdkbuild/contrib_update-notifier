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
 * File:   utils.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 9:56 PM
 */

#ifndef UPDATE_CHECKER_UTILS_HPP
#define	UPDATE_CHECKER_UTILS_HPP

#include <stdint.h>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <windows.h>
// http://stackoverflow.com/a/6884102/314015
#ifdef max
#undef max
#endif // max
#endif // _WIN32

#include "CheckerException.hpp"

namespace checker {
namespace utils {

template<typename T>
std::string to_string(const T& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
}

std::string strip_filename(const std::string& file_path);

uint32_t parse_uint32(const std::string& str);

uint64_t parse_uint64(const std::string& str);

bool starts_with(const std::string& value, const std::string& start);

bool ends_with(const std::string& value, const std::string& ending);

std::vector<std::string> split(const std::string& str, char delim);

std::string& pad_left(std::string& str, size_t len, char padding);

#ifdef _WIN32

std::wstring widen(const std::string& st);

std::string narrow(const wchar_t* wbuf, size_t length);

std::string narrow(std::wstring wstr);

std::string errcode_to_string(uint32_t code);

class NamedMutex {
  DWORD  error;
  HANDLE mutex;

public:
    NamedMutex(const std::wstring& name) {
        mutex = CreateMutexW(NULL, FALSE, name.c_str());
        error = GetLastError();
    }
   
    ~NamedMutex() {
        if (mutex) {
            CloseHandle(mutex);
        }
    }

    bool already_taken() {
        return (ERROR_ALREADY_EXISTS == error);
    }
};

#endif // _WIN32

} // namespace
}

#endif	/* UPDATE_CHECKER_UTILS_HPP */

