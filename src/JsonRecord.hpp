/* 
 * File:   JsonRecord.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 1:14 PM
 */

#ifndef UPDATE_CHECKER_JSONRECORD_HPP
#define	UPDATE_CHECKER_JSONRECORD_HPP

#include <stdint.h>
#include <limits>
#include <string>

#include "jansson.h"

#include "CheckerException.hpp"

namespace checker {

class JsonRecord {
    mutable json_t* json;
    
public:
    JsonRecord();
    
    JsonRecord(json_t* json);
    
    ~JsonRecord();
    
    // Pre-C++11 move logic
    JsonRecord(const JsonRecord& other);
    
    const json_t* get() const;
    
    const char* get_string(const std::string& fieldname, const std::string& defaultval = "") const;
        
    void put_string(const std::string& fieldname, const std::string& value);
    
    uint32_t get_uint32(const std::string& fieldname, uint32_t defaultval = 0) const;
    
    void put_uint32(const std::string& fieldname, uint32_t value);

    bool get_bool(const std::string& fieldname, bool defaultval = false) const;
    
    void put_bool(const std::string& fieldname, bool value);
    
private:    
    JsonRecord& operator=(const JsonRecord& other);
    
};

} // namespace

#endif	/* UPDATE_CHECKER_JSONRECORD_HPP */

