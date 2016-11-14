/* 
 * File:   utils.cpp
 * Author: alex
 *
 * Created on October 24, 2016, 7:23 PM
 */

#include "utils.hpp"

#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <windows.h>
#endif // _WIN32

#include <time.h>

namespace checker {
namespace utils {

std::string strip_filename(const std::string& file_path) {
    std::string::size_type pos = file_path.find_last_of("/\\");
    if (std::string::npos != pos && pos < file_path.length() - 1) {
        return std::string(file_path.data(), pos + 1);
    }
    return std::string(file_path.data(), file_path.length());
}

#ifdef _WIN32

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

} // namespace


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


#endif //_WIN32

} // namespace
}
