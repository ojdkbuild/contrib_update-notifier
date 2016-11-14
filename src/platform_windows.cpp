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

namespace { // anonymous

class WCharBuffer {
    wchar_t* buffer;

public:
    WCharBuffer(wchar_t* buffer):
    buffer(buffer) { }

    ~WCharBuffer() {
        if (buffer) {
            LocalFree(buffer);
        }
    }

    wchar_t* get() {
        return buffer;
    }

private:
    WCharBuffer(const WCharBuffer& other);
    
    WCharBuffer& operator=(const WCharBuffer& other);
};

std::string errcode_to_string(uint32_t code);

std::wstring widen(const std::string& st) {
    if (st.empty()) {
        return std::wstring();
    }
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, st.c_str(), static_cast<int> (st.length()), NULL, 0);
    if (0 == size_needed) {
        throw CheckerException(std::string() + "Error on string widen calculation," +
            " string: [" + st + "], error: [" + errcode_to_string(GetLastError()) + "]");
    }
    std::wstring res;
    res.resize(size_needed);
    int chars_copied = MultiByteToWideChar(CP_UTF8, 0, st.c_str(), static_cast<int> (st.size()), &res.front(), size_needed);
    if (chars_copied != size_needed) {
        throw CheckerException(std::string() + "Error on string widen execution," +
            " string: [" + st + "], error: [" + errcode_to_string(GetLastError()) + "]");
    }
    return res;
}

std::string narrow(const wchar_t* wbuf, size_t length) {
    if (0 == length) {
        return std::string();
    }
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int> (length), NULL, 0, NULL, NULL);
    if (0 == size_needed) {
        throw CheckerException(std::string() + "Error on string narrow calculation," +
            " string length: [" + utils::to_string(length) + "], error code: [" + utils::to_string(GetLastError()) + "]");
    }
    std::string res;
    res.resize(size_needed);
    int bytes_copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int> (length), &res.front(), size_needed, NULL, NULL);
    if (bytes_copied != size_needed) {
        throw CheckerException(std::string() + "Error on string narrow execution," +
            " string length: [" + utils::to_string(length) + "], error code: [" + utils::to_string(GetLastError()) + "]");
    }
    return res;
}

std::string narrow(std::wstring wstr) {
    return narrow(wstr.c_str(), wstr.length());
}

std::string errcode_to_string(uint32_t code) {
    if (0 == code) {
        return std::string();
    }
    wchar_t* buf_p = NULL;
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<wchar_t*> (&buf_p), 0, nullptr);
    if (0 == size) {
        return "Cannot format code: [" + utils::to_string(code) + "]" +
                " into message, error code: [" + utils::to_string(GetLastError()) + "]";
    }
    WCharBuffer buf(buf_p);
    if (size <= 2) {
        return "code: [" + utils::to_string(code) + "], message: []";
    }
    try {
        std::string msg = narrow(buf.get(), size - 2);
        return "code: [" + utils::to_string(code) + "], message: [" + msg + "]";
    } catch (const std::exception& e) {
        return "Cannot format code: [" + utils::to_string(code) + "]" +
                " into message, narrow error: [" + e.what() + "]";
    }
}


} // namespace

bool file_exists(const std::string& filepath) {
    std::wstring wpath = widen(filepath);
    DWORD res = GetFileAttributesW(wpath.c_str());
    return (INVALID_FILE_ATTRIBUTES != res && !(res & FILE_ATTRIBUTE_DIRECTORY));
}

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes) {
    std::wstring wpath = widen(filepath);
    std::wstring wmode = widen(mode);
    FILE* fd = _wfopen(wpath.c_str(), wmode.c_str());
    if (NULL == fd) {
        throw CheckerException(std::string() + "Cannot open file: [" + filepath + "]," + 
                " with mode: [" + mode + "], error: [" + errcode_to_string(GetLastError()) + "]");
    }
    return FileDescriptor(filepath, fd, max_read_bytes);
}

void close_file(FILE* file) {
    fclose(file);
}

std::string get_userdata_directory(const Config& cf) {
    return cf.work_directory; 
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
            " error: [" + errcode_to_string(GetLastError()) + "]");
    }
    return narrow(wst);
}

std::string current_datetime() {
    time_t cur = time(NULL);
    struct tm time;
    localtime_s(&time, &cur);
    return std::string(asctime(&time));
}

} // namespace
}
