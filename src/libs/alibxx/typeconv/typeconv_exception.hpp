#pragma once

#include <iostream>


namespace A_LIB_NAMESPACE{

class TypeConvException: public std::exception{
public:
    TypeConvException(const std::string & str) : message(str){}
    virtual ~TypeConvException() throw(){}

    virtual const char* what() const throw(){
        return message.c_str();
    }

private:
    std::string message;
};






}
