/* 
 * File:   JsonRecord.cpp
 * Author: alex
 *
 * Created on October 21, 2016, 1:49 PM
 */

#include "JsonRecord.hpp"

#include "utils.hpp"

namespace checker {

JsonRecord::JsonRecord() :
json(json_object()) {
    if (!json) {
        throw CheckerException("Cannot create empty JSON object");
    }
}

JsonRecord::JsonRecord(json_t* json) :
json(json) {
    if (!this->json) {
        throw CheckerException("Invalid 'null' JSON specified");
    }
    if (!json_is_object(this->json)) {
        throw CheckerException("Invalid 'non-object' JSON specified");
    }
}

JsonRecord::~JsonRecord() {
    if (json) {
        json_decref(json);
    }
}

/**
 * Pre-C++11 move logic
 * 
 * @param other
 */
JsonRecord::JsonRecord(const JsonRecord& other) :
json(other.json) {
    other.json = NULL;
}

const json_t* JsonRecord::get() const {
    return json;
}

const char* JsonRecord::get_string(const std::string& fieldname, const std::string& defaultval) const {
    json_t* field = json_object_get(json, fieldname.c_str());
    if (!(field && json_is_string(field))) {
        return defaultval.c_str();
    }
    return json_string_value(field);
}

void JsonRecord::put_string(const std::string& fieldname, const std::string& value) {
    json_t* field = json_string(value.c_str());
    if (!field) {
        throw CheckerException("Cannot create JSON string, field: [" + fieldname + "]," +
                " value: [" + value +"]");
    }
    int err = json_object_set(json, fieldname.c_str(), field);
    if (err) {
        throw CheckerException("Cannot set JSON string, field: [" + fieldname + "]," + 
                " value: [" + value + "]");
    }
}

uint32_t JsonRecord::get_uint32(const std::string& fieldname, uint32_t defaultval) const {
    json_t* field = json_object_get(json, fieldname.c_str());
    if (!(field && json_is_integer(field))) {
        return defaultval;
    }
    json_int_t res = json_integer_value(field);
    if (res < 0) {
        return defaultval;
    }
    uint64_t res_u64 = static_cast<uint64_t> (res);
    if (res >= std::numeric_limits<uint32_t>::max()) {
        return defaultval;
    }
    return static_cast<uint32_t> (res_u64);
}

void JsonRecord::put_uint32(const std::string& fieldname, uint32_t value) {
    json_t* field = json_integer(value);
    if (!field) {
        throw CheckerException("Cannot create JSON integer, field: [" + fieldname + "]," +
                " value: [" + utils::to_string(value) + "]");
    }
    int err = json_object_set(json, fieldname.c_str(), field);
    if (err) {
        throw CheckerException("Cannot set JSON integer, field: [" + fieldname + "]," +
                " value: [" + utils::to_string(value) + "]");
    }
}

bool JsonRecord::get_bool(const std::string& fieldname, bool defaultval = false) const {
    json_t* field = json_object_get(json, fieldname.c_str());
    if (!(field && json_is_boolean(field))) {
        return defaultval;
    }
    return json_is_true(field) ? true : false;
}

void JsonRecord::put_bool(const std::string& fieldname, bool value) {
    json_t* field = json_boolean(value);
    if (!field) {
        throw CheckerException("Cannot create JSON bool, field: [" + fieldname + "]," +
                " value: [" + utils::to_string(value) + "]");
    }
    int err = json_object_set(json, fieldname.c_str(), field);
    if (err) {
        throw CheckerException("Cannot set JSON bool, field: [" + fieldname + "]," +
                " value: [" + utils::to_string(value) + "]");
    }
}

} // namespace

