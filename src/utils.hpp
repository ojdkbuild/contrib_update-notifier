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
 * File:   utils.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 9:56 PM
 */

#ifndef UPDATE_CHECKER_UTILS_HPP
#define	UPDATE_CHECKER_UTILS_HPP

#include <cstdint>
#include <sstream>
#include <string>

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

#ifdef _WIN32

std::wstring widen(const std::string& st);

std::string narrow(const wchar_t* wbuf, size_t length);

std::string narrow(std::wstring wstr);

std::string errcode_to_string(uint32_t code);

#endif // _WIN32

} // namespace
}

#endif	/* UPDATE_CHECKER_UTILS_HPP */

