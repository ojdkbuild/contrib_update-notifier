/* 
 * File:   jsonio.cpp
 * Author: alex
 * 
 * Created on October 20, 2016, 11:00 PM
 */

#include "jsonio.hpp"

#include "jansson.h"

#include "CheckerException.hpp"
#include "FileDescriptor.hpp"
#include "platform.hpp"
#include "utils.hpp"

namespace checker {

JsonRecord read_from_file(const Config& cf, const std::string& filepath) {    
    FileDescriptor fd = platform::open_file(filepath, "r");
    json_error_t error;
    int flags = JSON_REJECT_DUPLICATES | JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK;
    json_t json = json_load_callback(FileDescriptor::read_cb, &fd, flags, &error);
    if (!json) {
        throw CheckerException("Error parsing JSON:" +
            " text: [" + error.text + "]" +
            " line: [" + utils::to_string(error.line) + "]" +
            " column: [" + utils::to_string(error.column) + "]" +
            " position: [" + utils::to_string(error.position) + "],"
            " callback error: [" + fd.get_error() + "]");
    }
    return JsonRecord(json);
}

void write_to_file(const Config& cf, const JsonRecord& json, const std::string& filepath) {
    FileDescriptor fd = platform::open_file(filepath, "w");
    int flags = JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_PRESERVE_ORDER;
    int res = json_dump_callback(json.get(), FileDescriptor::write_cb, &fd, flags);
    if (0 != res) {
        throw CheckerException("Error dumping JSON: [" + fd.get_error() + "]");
    }
}

} // namespace
