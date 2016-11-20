/*
 * Copyright 2016, akashche at redhat.com
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
 * File:   CheckerException.hpp
 * Author: alex
 *
 * Created on October 21, 2016, 11:34 AM
 */

#ifndef UPDATE_CHECKER_CHECKEREXCEPTION_HPP
#define	UPDATE_CHECKER_CHECKEREXCEPTION_HPP

#include <exception>
#include <string>

namespace checker {

/**
 * Exception class for checker errors
 */
class CheckerException : public std::exception {
protected:
    /**
     * Error message
     */
    std::string message;

public:
    /**
     * Default constructor
     */
    CheckerException() { };

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    CheckerException(const std::string& message) :
    message(message.data(), message.length()) { }

    ~CheckerException() throw() {}
        
    /**
     * Returns error message
     * 
     * @return error message
     */
    virtual const char* what() const throw() {
        return message.c_str();
    }
};

} // namespace

#endif	/* UPDATE_CHECKER_CHECKEREXCEPTION_HPP */

