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

