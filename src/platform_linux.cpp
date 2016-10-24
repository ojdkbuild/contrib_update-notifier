/* 
 * File:   platform_linux.cpp
 * Author: alex
 *
 * Created on October 21, 2016, 10:23 PM
 */

#include "platform.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "CheckerException.hpp"
#include "utils.hpp"

namespace checker {
namespace platform {

bool file_exists(const std::string& filepath) {
    struct stat st;
    return 0 == stat(filepath.c_str(), &st);
}

FileDescriptor open_file(const std::string& filepath, const std::string& mode, uint32_t max_read_bytes) {
    FILE* res = fopen(filepath.c_str(), mode.c_str());
    if (!res) {
        throw CheckerException("Error opening file, path: [" + filepath + "]," + 
                " mode: [" + mode + "], error: [" + strerror(errno) +
                " (" + utils::to_string(errno) + ")]");
    }
    return FileDescriptor(filepath, res, max_read_bytes);
}

void close_file(FILE* file) {
    int err = fclose(file);
    (void) err; // may be called from destructor
}

std::string get_userdata_directory(const Config& cf) {
    struct passwd pw;
    struct passwd* pwp;
    std::memset(&pw, '\0', sizeof(pw));
    std::string buf;
    buf.resize(cf.max_path_length);
    errno = ENOENT;
    int err = getpwuid_r(getuid(), &pw, &buf[0], cf.max_path_length, &pwp);
    if (err) {
        throw CheckerException(std::string() + "Error getting appdata directory: [" + strerror(errno) +
                " (" + utils::to_string(errno) + ")]");
    }
    if (!pw.pw_dir) {
        throw CheckerException(std::string() + "Error getting home directory for user," + 
                " uid: [" + utils::to_string(getuid()) + "]");
    }
    return std::string(pw.pw_dir) + "/.config/";
}

void create_directory(const std::string& dirpath) {
    int err = mkdir(dirpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err < 0 && EEXIST != errno) {
        throw CheckerException(std::string() + "Error creating directory: [" + dirpath + "]," +
                " error: [" + strerror(errno) + " (" + utils::to_string(errno) + ")]");
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

std::string current_executable_path() {
    std::string res;
    ssize_t size = 64;
    for (;;) {
        res.resize(size);
        char* link = &res[0];
        ssize_t res_size = readlink("/proc/self/exe", link, size);
        if (res_size < 0) {
            throw CheckerException(strerror(errno));
        }
        if (res_size < size) {
            res.resize(res_size);
            break;
        }
        size = size * 2;
    }
    return res;
}

std::string current_datetime() {
    time_t cur = time(NULL);
    struct tm time;
    localtime_r(&cur, &time);
    return std::string(asctime(&time));
}

} // namespace
}
