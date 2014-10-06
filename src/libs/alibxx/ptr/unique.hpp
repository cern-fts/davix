#ifndef PTR_UNIQUE_HPP
#define PTR_UNIQUE_HPP

#include <memory>
#include "../alibxx.hpp"

namespace A_LIB_NAMESPACE{

namespace Ptr{


template <typename T>
class Scoped{
public:
    typedef T element_type;

    explicit Scoped(): _ptr(NULL){}
    explicit Scoped(T* p) : _ptr(p){}
    ~Scoped(){ delete _ptr;}

    inline T* release(){
        T* r= _ptr;
        _ptr = NULL;
        return r;
    }

    inline void reset(T* p = NULL){
        Scoped<T> tmp(p);
        swap(tmp);
    }

    T& operator*(){
        return *_ptr;
    }

    T* operator->(){
        return _ptr;
    }

    bool operator==(const Scoped<T> & other){
        return (_ptr == other._ptr);
    }

    inline T* get(){
        return _ptr;
    }

    inline void swap(Scoped<T> & other){
         std::swap(other._ptr, _ptr);
    }

private:
    T* _ptr;
    Scoped(const Scoped<T> & other);
    Scoped & operator=(const Scoped<T> & other);
};




}

}




#endif // UNIQUE_HPP
