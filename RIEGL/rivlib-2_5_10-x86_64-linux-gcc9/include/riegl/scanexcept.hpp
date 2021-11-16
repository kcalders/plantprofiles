// $Id$

#ifndef SCANEXCEPT_HPP
#define SCANEXCEPT_HPP

#include <exception>
#include <string>

namespace scanlib {

    class scanlib_exception
        : public std::exception {

    protected:
        std::string msg;

    public:
        static const std::string default_msg_arg;

        scanlib_exception(
            const std::string&   what_arg = default_msg_arg
            , const std::string& file_arg = ""
            , unsigned           line_arg = 0
        );

        virtual ~scanlib_exception() throw() {}

        virtual const char*
        what() const throw();
    };
}

#endif //SCANEXCEPT_HPP
