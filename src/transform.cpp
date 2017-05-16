/*
 * Copyright 2017 Red Hat, Inc.
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
 * File:   transform.cpp
 * Author: alex
 *
 * Created on May 15, 2017, 1:41 PM
 */

#include "transform.hpp"

#include <stdint.h>
#include <string>
#include <vector>

#include "utils.hpp"

namespace checker {

namespace { // anonymous

class JsonHolder {
    json_t* json;

    JsonHolder(const JsonHolder&);
    
    JsonHolder& operator=(const JsonHolder&);
    
public:
    JsonHolder(json_t* json):
    json(json) {
        if (!json) {
            throw CheckerException("Cannot create empty JSON object");
        }
    }
    
    ~JsonHolder() {
        if (json) {
            json_decref(json);
        }
    }
    
    json_t* get() {
        return json;
    }
    
    json_t* release() {
        json_t* res = this->json;
        this->json = NULL;
        return res;
    }
    
    void put_string(const std::string& fieldname, const std::string& value) {
        json_t* field = json_string(value.c_str());
        if (!field) {
            throw CheckerException("Cannot create JSON string, field: [" + fieldname + "]," +
                    " value: [" + value + "]");
        }
        int err = json_object_set_new(json, fieldname.c_str(), field);
        if (err) {
            throw CheckerException("Cannot set JSON string, field: [" + fieldname + "]," +
                    " value: [" + value + "]");
        }
    }

    void put_uint64(const std::string& fieldname, uint64_t value) {
        json_t* field = json_integer(static_cast<json_int_t> (value));
        if (!field) {
            throw CheckerException("Cannot create JSON integer, field: [" + fieldname + "]," +
                    " value: [" + utils::to_string(value) + "]");
        }
        int err = json_object_set_new(json, fieldname.c_str(), field);
        if (err) {
            throw CheckerException("Cannot set JSON integer, field: [" + fieldname + "]," +
                    " value: [" + utils::to_string(value) + "]");
        }
    }
};

class Ver {
public:    
    uint32_t ver_major;
    uint32_t ver_update;
    uint32_t ver_build;
    uint32_t ver_release;
    std::string os_arch;
    
    Ver() :
    ver_major(0),
    ver_update(0),
    ver_build(0),
    ver_release(0) { }
    
    Ver(const Ver& other) :
    ver_major(other.ver_major),
    ver_update(other.ver_update),
    ver_build(other.ver_build),
    ver_release(other.ver_release),
    os_arch(other.os_arch) { }
    
    // 1.8.0.131-1.b11
    std::string ver_string() {
        return "1." + utils::to_string(ver_major) + ".0." +
                utils::to_string(ver_update) + "-" + utils::to_string(ver_release) + ".b" +
                utils::to_string(ver_build);
    }
    
    // 10813101011
    uint64_t ver_number() {
        std::string majorst = utils::to_string(ver_major);
        utils::pad_left(majorst, 2, '0');
        std::string updatest = utils::to_string(ver_update);
        utils::pad_left(updatest, 3, '0');
        std::string releasest = utils::to_string(ver_release);
        utils::pad_left(releasest, 2, '0');
        std::string buildst = utils::to_string(ver_build);
        utils::pad_left(buildst, 3, '0');
        return utils::parse_uint64("1" + majorst + updatest + releasest + buildst);
    }
    
private:
    Ver operator=(const Ver&);
};

std::string json_get_string(json_t* json, const std::string& fieldname, const std::string& defaultval) {
    json_t* field = json_object_get(json, fieldname.c_str());
    if (!(field && json_is_string(field))) {
        return defaultval;
    }
    const char* val = json_string_value(field);
    return std::string(val);
}

json_t* extract_featured_artifact(json_t* json) {
    static const char* err = "Cannot extract 'featuredArtifact' from DM response";
    if (0 == json_array_size(json)) {
        throw CheckerException(err);
    }
    json_t* obj = json_array_get(json, 0);
    if (!obj || !json_is_object(obj)) {
        throw CheckerException(err);
    }
    json_t* ft = json_object_get(obj, "featuredArtifact");
    if (!ft || !json_is_object(ft)) {
        throw CheckerException(err);
    }
    return ft;
}

uint32_t extract_major(const std::vector<std::string>& verst_dot_chunks) {
    uint32_t res = utils::parse_uint32(verst_dot_chunks.at(0));
    if (1 == res) {
        return utils::parse_uint32(verst_dot_chunks.at(1));
    }
    return res;
}

uint32_t extract_build(const std::vector<std::string>& post_dot_chunks) {
    const std::string& buildst = post_dot_chunks.at(1);
    if (buildst.length() < 2) {
        throw CheckerException("Cannot extract 'build' from DM response");
    }
    return utils::parse_uint32(buildst.substr(1));
}

Ver extract_version(json_t* json) {
    static const char* err = "Cannot extract version from DM response";
    // https://developers.redhat.com/download-manager/file/java-1.8.0-openjdk-1.8.0.131-1.b11.redhat.windows.x86_64.msi
    std::string url = json_get_string(json, "url", "");
    if (url.empty()) {
        throw CheckerException(err);
    }
    if (!utils::ends_with(url, ".msi")) {
        throw CheckerException(err);
    }
    
    std::vector<std::string> slash_chunks = utils::split(url, '/');
    if (slash_chunks.size() < 3) {
        throw CheckerException(err);
    }
    // java-1.8.0-openjdk-1.8.0.131-1.b11.redhat.windows.x86_64.msi
    const std::string& name = slash_chunks.back();
    if(!utils::starts_with(name, "java-") || !utils::ends_with(name, ".msi")) {
        throw CheckerException(err);
    }
    
    std::vector<std::string> dash_chunks = utils::split(name, '-');
    if (5 != dash_chunks.size()) {
        throw CheckerException(err);
    }
    
    // 1.8.0.13
    const std::string& verst = dash_chunks.at(3);
    std::vector<std::string> verst_dot_chunks = utils::split(verst, '.');
    if (verst_dot_chunks.size() < 3) {
        throw CheckerException(err);
    }
    
    // 1.b11.redhat.windows.x86_64.msi
    const std::string& postfix = dash_chunks.back();
    std::vector<std::string> post_dot_chunks = utils::split(postfix, '.');
    if (post_dot_chunks.size() < 5) {
        throw CheckerException(err);
    }
    
    Ver ver;
    ver.ver_major = extract_major(verst_dot_chunks);
    ver.ver_update = utils::parse_uint32(verst_dot_chunks.back());
    ver.ver_build = extract_build(post_dot_chunks);
    ver.ver_release = utils::parse_uint32(post_dot_chunks.front());
    ver.os_arch = post_dot_chunks.at(post_dot_chunks.size() - 2);
    return ver;
}

} // namespace

json_t* download_manager_transform(json_t* dmjson) {
    if(!dmjson || !json_is_array(dmjson)) {
        return dmjson;
    }
    JsonHolder dm(dmjson);
    JsonHolder json(json_object());
    
    json_t* featured_artifact = extract_featured_artifact(dm.get());
    Ver ver = extract_version(featured_artifact);
    
    json.put_string("package_name", "java-1." + utils::to_string(ver.ver_major) + ".0-openjdk");
    json.put_string("package_description", "OpenJDK runtime");
    json.put_string("os_name", "Windows");
    json.put_string("os_arch", ver.os_arch);
    json.put_string("version_string", ver.ver_string());
    json.put_uint64("version_number", ver.ver_number());
    json.put_string("ui_balloon_text", "OpenJDK version " + ver.ver_string() + " is available for download");
    json.put_string("ui_update_header", "OpenJDK version " + ver.ver_string() + " is available for download");
    json.put_string("ui_update_text", "This version contains a number of security fixes.\n\n"
            "To proceed with download and installation please follow a link above, download page will be opened inside your web-browser.\n\n"
            "To change a schedule of this notification please see 'ojdkbuild_jdk_update_checker' and 'ojdkbuild_jdk_update_notifier' tasks in 'Task Scheduler'.\n\n"
            "To disable this notification permanently please select the installed application in 'Programs and Features' with a right-click, choose 'Change' and disable 'Update Notifier' installation feature.");
    
    return json.release();
}

} // namespace

