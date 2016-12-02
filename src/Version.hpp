/*
 * Copyright 2016 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   Version.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:13 PM
 */

#ifndef UPDATE_CHECKER_VERSION_HPP
#define	UPDATE_CHECKER_VERSION_HPP

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
    uint64_t version_number;
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
    version_number(json.get_uint64("version_number")),
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
        json.put_uint64("version_number", version_number);
        json.put_string("ui_balloon_text", ui_balloon_text);
        json.put_string("ui_update_header", ui_update_header);
        json.put_string("ui_update_text", ui_update_text);
        return json;
    }
};

} // namespace

#endif	/* UPDATE_CHECKER_VERSION_HPP */

