#pragma once

#include <iostream>


namespace A_LIB_NAMESPACE{

class ChronoException: public std::exception{
public:
    ChronoException(const std::string & str) : message(str){}
    virtual ~ChronoException() throw(){}

    virtual const char* what() const throw(){
        return message.c_str();
    }

private:
    std::string message;
};






}
