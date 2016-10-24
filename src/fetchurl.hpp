/* 
 * File:   fetchurl.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:00 PM
 */

#ifndef UPDATE_CHECKER_FETCHURL_HPP
#define	UPDATE_CHECKER_FETCHURL_HPP

/**
 * single public fetchurl function
 * takes Config and localVersion
 * returns remoteVersion
 */

#include "CheckerException.hpp"
#include "Config.hpp"
#include "JsonRecord.hpp"

namespace checker {

JsonRecord fetchurl(const Config& cf);

} // namespace

#endif	/* UPDATE_CHECKER_FETCHURL_HPP */

