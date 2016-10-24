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

bool file_exists(const std::string& filepath);

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes);

void close_file(FILE* file);

std::string get_userdata_directory(const Config& cf);

void create_directory(const std::string& dirpath);

void thread_sleep_millis(uint32_t millis);

std::string current_executable_path();

std::string current_datetime();

} // namespace
} 

#endif	/* UPDATE_CHECKER_PLATFORM_HPP */

