/* 
 * File:   jsonio.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:00 PM
 */

#ifndef UPDATE_CHECKER_JSONIO_HPP
#define	UPDATE_CHECKER_JSONIO_HPP

#include "Config.hpp"
#include "JsonRecord.hpp"

namespace checker {

JsonRecord read_from_file(const std::string& filepath, uint32_t max_read_bytes);

void write_to_file(const JsonRecord& json, const std::string& filepath);

} // namespace

#endif	/* UPDATE_CHECKER_JSONIO_HPP */

