#ifndef DAVOPS_HPP
#define DAVOPS_HPP

#include <memory>

#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>

#include <neon/neonsession.hpp>

namespace Davix{

class WebdavQuery{
public:
    WebdavQuery(Context & c);


    int davDelete(const RequestParams * params, const Uri & uri, DavixError** err);

private:
    WebdavQuery(const WebdavQuery &);
    WebdavQuery & operator=(const WebdavQuery &);
    Context & _c;
};




}

#endif // DAVOPS_HPP
