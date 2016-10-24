/* 
 * File:   platform.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:31 PM
 */

#ifndef UPDATE_CHECKER_PLATFORM_HPP
#define	UPDATE_CHECKER_PLATFORM_HPP

#include <cstdio>
#include <string>

#include "Config.hpp"
#include "FileDescriptor.hpp"

/**
 * SHGetKnownFolderPath
 */

namespace checker {
namespace platform {

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes);

void close_file(FILE* file);

std::string get_appdata_directory(const Config& cf);

void create_directory(const std::string& dirpath);

void thread_sleep_millis(uint32_t millis);

} // namespace
} 

#endif	/* UPDATE_CHECKER_PLATFORM_HPP */

