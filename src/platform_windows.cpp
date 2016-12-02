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
 * File:   platform_windows.cpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:32 PM
 */

#include "platform.hpp"

#include <cstdint>
#include <ctime>
#include <algorithm>
#include <string>

#include <windows.h>
#include <shlobj.h>

#include "CheckerException.hpp"
#include "utils.hpp"

namespace checker {
namespace platform {

namespace { // anonymous

class WCharCoBuffer {
    wchar_t* buffer;

public:
    WCharCoBuffer(wchar_t* buffer):
    buffer(buffer) { }

    ~WCharCoBuffer() {
        if (buffer) {
            CoTaskMemFree(buffer);
        }
    }

    wchar_t* get() {
        return buffer;
    }

private:
    WCharCoBuffer(const WCharCoBuffer& other);
    
    WCharCoBuffer& operator=(const WCharCoBuffer& other);
};

} // namespace


bool file_exists(const std::string& filepath) {
    std::wstring wpath = utils::widen(filepath);
    DWORD res = GetFileAttributesW(wpath.c_str());
    return (INVALID_FILE_ATTRIBUTES != res && !(res & FILE_ATTRIBUTE_DIRECTORY));
}

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes) {
    std::wstring wpath = utils::widen(filepath);
    std::wstring wmode = utils::widen(mode);
    FILE* fd = _wfopen(wpath.c_str(), wmode.c_str());
    if (NULL == fd) {
        throw CheckerException(std::string() + "Cannot open file: [" + filepath + "]," + 
                " with mode: [" + mode + "], error: [" + utils::errcode_to_string(GetLastError()) + "]");
    }
    return FileDescriptor(filepath, fd, max_read_bytes);
}

void close_file(FILE* file) {
    fclose(file);
}

std::string get_userdata_directory() {
    wchar_t* wpath = NULL;
    HRESULT err = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &wpath);
    if (S_OK != err) {
        throw CheckerException(std::string() + "Error getting userdata dir");
    }
    WCharCoBuffer buf(wpath);
    std::string path = utils::narrow(buf.get());
    std::replace(path.begin(), path.end(), '\\', '/');
    path.push_back('/');
    return path;
}

void create_directory(const std::string& dirpath) {
    std::wstring wpath = utils::widen(dirpath);
    BOOL err = CreateDirectoryW(&wpath.front(), NULL);
    if (0 == err && ERROR_ALREADY_EXISTS != GetLastError()) {
        throw CheckerException(std::string() + "Error getting creating dir," +
            " path: [" + dirpath + "], error: [" + utils::errcode_to_string(GetLastError()) + "]");
    }
}

void delete_file(const std::string& dirpath) {
    std::wstring wpath = utils::widen(dirpath);
    BOOL err = DeleteFileW(wpath.c_str());
    (void) err;
}

void delete_directory(const std::string& dirpath) {
    std::wstring wpath = utils::widen(dirpath);
    BOOL err = RemoveDirectoryW(wpath.c_str());
    (void) err;
}

void thread_sleep_millis(uint32_t millis) {
    Sleep(millis);
}

std::string current_executable_path() {
    std::wstring wst;
    wst.resize(MAX_PATH);
    int success = GetModuleFileNameW(NULL, &wst.front(), static_cast<DWORD>(wst.length()));
    if (0 == success) {
        throw CheckerException(std::string() + "Error getting current executable dir," +
            " error: [" + utils::errcode_to_string(GetLastError()) + "]");
    }
    return utils::narrow(wst);
}

std::string current_datetime() {
    time_t cur = time(NULL);
    struct tm time;
    localtime_s(&time, &cur);
    char tmpbuf[128];
    strftime(tmpbuf, 128, "%Y-%m-%d_%H:%M:%S", &time);
    return std::string(tmpbuf);
}

} // namespace
}
