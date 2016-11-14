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

std::string current_datetime();

#ifdef _WIN32

std::wstring widen(const std::string& st);

std::string narrow(const wchar_t* wbuf, size_t length);

std::string narrow(std::wstring wstr);

std::string errcode_to_string(uint32_t code);

#endif // _WIN32

} // namespace
}

#endif	/* UPDATE_CHECKER_UTILS_HPP */

