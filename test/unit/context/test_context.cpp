
#include <davix_cpp.hpp>
#include <davixcontext.hpp>
#include <davixrequestparams.hpp>
#include <gtest/gtest.h>


// instanciate and play with gates
TEST(ContextTest, CreateDelete){
    Davix::Context c1;

    Davix::Context* c2 = c1.clone();
    ASSERT_TRUE(c2 != NULL);
    Davix::Context* c3 = c1.clone();


    delete c2;
    delete c3;

}




TEST(RequestParametersTest, CreateDelete){
    Davix::RequestParams params;

    ASSERT_EQ(params.getSSLCACheck(), true);
    ASSERT_EQ(params.getOperationTimeout()->tv_sec, DAVIX_DEFAULT_OPS_TIMEOUT);
    ASSERT_EQ(params.getConnexionTimeout()->tv_sec, DAVIX_DEFAULT_CONN_TIMEOUT);
    ASSERT_EQ(params.getAuthentificationCallbackData(), (void*) NULL);
    ASSERT_EQ(params.getAuthentificationCallbackFunction(), (int (*)(davix_auth_st*, const davix_auth_info_t*, void*, Davix_error**)) NULL);

    params.setSSLCAcheck(false);
    struct timespec timeout_co, timeout_ops;
    timeout_co.tv_sec =10;
    timeout_co.tv_nsec=99;
    timeout_ops.tv_sec=20;
    timeout_ops.tv_nsec=0xFF;

    ASSERT_EQ(params.getSSLCACheck(), false);
    params.setOperationTimeout(&timeout_co);
    ASSERT_EQ(params.getOperationTimeout()->tv_sec, 10);
    params.setConnexionTimeout(&timeout_ops);
    ASSERT_EQ(params.getConnexionTimeout()->tv_sec, 20);

    Davix::RequestParams p2(&params);
    Davix::RequestParams p3(p2);

    ASSERT_EQ(p2.getOperationTimeout()->tv_sec, 10);
    ASSERT_EQ(p3.getOperationTimeout()->tv_sec, 10);

    ASSERT_EQ(p2.getConnexionTimeout()->tv_sec, 20);
    ASSERT_EQ(p3.getConnexionTimeout()->tv_sec, 20);

}


TEST(DavixErrorTest, CreateDelete){
    Davix::DavixError err("test_dav_scope", Davix::StatusCode::isNotADirectory, " problem");
    ASSERT_EQ(err.getErrMsg(), " problem");
    ASSERT_EQ(err.getStatus(), Davix::StatusCode::isNotADirectory);
    ASSERT_EQ(err.getStatus(), DAVIX_STATUS_IS_NOT_A_DIRECTORY);

    Davix::DavixError * err2=NULL;
    Davix::DavixError::setupError(&err2,"test_dav_scope2", Davix::StatusCode::ConnexionProblem, "connexion problem");
    ASSERT_EQ(err2->getErrMsg(), "connexion problem");
    ASSERT_EQ(err2->getStatus(), Davix::StatusCode::ConnexionProblem);

    Davix::DavixError::clearError(&err2);
    Davix::DavixError::clearError(&err2);
}


