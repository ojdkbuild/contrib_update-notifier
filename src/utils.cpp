/* 
 * File:   utils.cpp
 * Author: alex
 *
 * Created on October 24, 2016, 7:23 PM
 */

#include "utils.hpp"

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

} // namespace
}
