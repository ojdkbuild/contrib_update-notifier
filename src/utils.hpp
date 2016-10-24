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

} // namespace
}

#endif	/* UPDATE_CHECKER_UTILS_HPP */

