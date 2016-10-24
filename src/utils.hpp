/* 
 * File:   utils.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 9:56 PM
 */

#ifndef UPDATE_CHECKER_UTILS_HPP
#define	UPDATE_CHECKER_UTILS_HPP

#include <sstream>
#include <string>

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

} // namespace
}

#endif	/* UPDATE_CHECKER_UTILS_HPP */

