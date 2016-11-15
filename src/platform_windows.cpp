/* 
 * File:   platform_windows.cpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:32 PM
 */

#include "platform.hpp"

#include <cstdint>
#include <ctime>
#include <string>

#include <windows.h>

#include "CheckerException.hpp"
#include "utils.hpp"

namespace checker {
namespace platform {


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

std::string get_userdata_directory(const Config& cf) {
    throw CheckerException("Unsupported operation: [get_userdata_directory]");
}

void create_directory(const std::string& dirpath) {
    throw CheckerException("Unsupported operation: [create_directory]");
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
    return std::string(asctime(&time));
}

} // namespace
}
