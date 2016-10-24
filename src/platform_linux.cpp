/* 
 * File:   platform_linux.cpp
 * Author: alex
 *
 * Created on October 21, 2016, 10:23 PM
 */

#include "platform.hpp"

#include <cerrno>
#include <cstdio>

#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "CheckerException.hpp"
#include "utils.hpp"

namespace checker {
namespace platform {

FileDescriptor open_file(const std::string& filepath, const std::string& mode) {
    FILE* res = fopen(filepath.c_str(), mode.c_str());
    if (!res) {
        throw CheckerException("Error opening file, path: [" + filepath + "]," + 
                " mode: [" + mode + "], error: [" + strerror(errno) +
                " (" + utils::to_string(errno) + ")]");
    }
    return FileDescriptor(res);
}

void close_file(FILE* file) {
    int err = fclose(file);
    (void) err; // may be called from destructor
    // if (err) {
    //    throw CheckerException("Error closing file: [" + strerror(errno) +
    //            " (" + utils::to_string(errno) + ")]");
    // }
}

std::string get_appdata_directory(const Config& cf) {
    struct passwd pw;
    struct passwd* pwp;
    std::memset(&pw, '\0', sizeof(pw));
    std::string buf;
    buf.resize(cf.max_path_length);
    int err = getpwuid_r(getuid(), &pw, &buf.front(), cf.max_path_length, &pwp);
    if (!err) {
        throw CheckerException("Error getting appdata directory: [" + strerror(errno) +
                " (" + utils::to_string(errno) + ")]");
    }
    if (!pw->pw_dir) {
        throw CheckerException("Error getting home directory for user," + 
                " uid: [" + utils::to_string(getuid()) + "]");
    }
    return std::string(pw->pw_dir) + "/.config/" + cf.application_name + "/";
}

void create_directory(const std::string& dirpath) {
    int err = mkdir(dirpath.c_str(), 0);
    if (err < 0 && EEXIST != errno) {
        throw CheckerException("Error creating directory: [" + strerror(errno) +
                " (" + utils::to_string(errno) + ")]");
    }
}

void thread_sleep_millis(uint32_t millis) {
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    int err = nanosleep(&ts, NULL);
    if (err < 0) {
        throw CheckerException("'nanosleep' error for mllis: [" + utils::to_string(millis) + "]," +
                " [" + strerror(errno) + " (" + utils::to_string(errno) + ")]");
    }
}

} // namespace
}
