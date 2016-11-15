/* 
 * File:   Version.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:13 PM
 */

#ifndef UPDATE_CHECKER_VERSION_HPP
#define	UPDATE_CHECKER_VERSION_HPP

/**
 * name
 * os
 * arch
 * version string
 * version number (int)
 * description (for bubble)
 */

#include <stdint.h>
#include <string>

#include "JsonRecord.hpp"

namespace checker {

class Version {
public:    
    std::string package_name;
    std::string package_description;
    std::string os_name;
    std::string os_arch;
    std::string version_string;
    uint32_t version_number;
    std::string ui_balloon_text;
    std::string ui_update_header;
    std::string ui_update_text;

    Version() :
    version_number(0) { }
    
    Version(const JsonRecord& json) :
    package_name(json.get_string("package_name")),
    package_description(json.get_string("package_description")),
    os_name(json.get_string("os_name")),
    os_arch(json.get_string("os_arch")),
    version_string(json.get_string("version_string")),
    version_number(json.get_uint32("version_number")),
    ui_balloon_text(json.get_string("ui_balloon_text")),
    ui_update_header(json.get_string("ui_update_header")),
    ui_update_text(json.get_string("ui_update_text")) { }
    
    JsonRecord to_json() {
        JsonRecord json;
        json.put_string("package_name", package_name);
        json.put_string("package_description", package_description);
        json.put_string("os_name", os_name);
        json.put_string("os_arch", os_arch);
        json.put_string("version_string", version_string);
        json.put_uint32("version_number", version_number);
        json.put_string("ui_balloon_text", ui_balloon_text);
        json.put_string("ui_update_header", ui_update_header);
        json.put_string("ui_update_text", ui_update_text);
        return json;
    }
};

} // namespace

#endif	/* UPDATE_CHECKER_VERSION_HPP */

