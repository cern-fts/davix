#ifndef DAVIX_HOOKS_HPP
#define DAVIX_HOOKS_HPP

#include <typeinfo>
#include <request/httprequest.hpp>

namespace Davix{

struct HookIntern;

///
/// \brief The HookTraits class
///
///  Base class for Daivx Hook functions
///
struct HookTraits{
    HookTraits();
    virtual ~HookTraits();

    virtual std::type_info & getType() =0;

    // internal
    HookIntern* d_ptr;
};


template <class T>
struct Hook: HookTraits{
    virtual std::type_info & getType(){
        return typeid(*static_cast<T>(this));
    }
};


typedef void (*CallbackHeader)(HttpRequest &, const std::string &, std::string &);

///
/// \brief Hook for HTTP Request header sending
///
struct HookSendHeader : public Hook<HookSendHeader>{
    CallbackHeader hook;
};


///
/// \brief Hook for HTTP Request header reception
///
struct HookReceiveHeader : public Hook<HookReceiveHeader>{
    CallbackHeader hook;
};


typedef void (*CallbackRequestExec)(HttpRequest &);


///
/// \brief Hook triggered before any Request execution
///
struct HookRequestPreExec : public Hook<HookRequestPreExec>{
    CallbackRequestExec hook;
};


///
/// \brief Hook triggered after any Request execution
///
struct HookRequestPostExec : public Hook<HookRequestPostExec>{
    CallbackRequestExec hook;
};






}


#endif // DAVIX_HOOKS_HPP
