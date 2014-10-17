#pragma once


#include "alibxx.hpp"

namespace A_LIB_NAMESPACE{

namespace Operator {



// need operator ==
template<typename T, typename O>
class Equality{
public:
     friend bool operator!=(const T& x, const O& y) { return !static_cast<bool>(x == y); }
};


// need operator == and <
template<typename T, typename O>
class Compare{
public:
    friend bool operator>(const T& x, const O& y)  { return y < x; }
    friend bool operator<=(const T& x, const O& y) { return !static_cast<bool>(y < x); }
    friend bool operator>=(const T& x, const O& y) { return !static_cast<bool>(x < y); }


};



// need operator +=
template<typename T, typename O>
class Addable{
public:
    friend T operator+(const T& x, const O& y)  { T res(x); res+= y; return res; }

};

template<typename T, typename O>
class Subtractable{
public:
    friend T operator-(const T& x, const O& y)  { T res(x); res-= y; return res; }

};




}


} // A_LIB_NAMESPACE
