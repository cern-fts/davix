#include <davix_internal_config.hpp>
#include <davix.hpp>
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
    ASSERT_EQ(params.getConnectionTimeout()->tv_sec, DAVIX_DEFAULT_CONN_TIMEOUT);
    ASSERT_TRUE((params.getClientCertCallbackX509().second == NULL));
    ASSERT_TRUE((params.getClientCertCallbackX509().first == NULL));
    ASSERT_TRUE( params.getTransparentRedirectionSupport());

    params.setSSLCAcheck(false);
    struct timespec timeout_co, timeout_ops;
    timeout_co.tv_sec =10;
    timeout_co.tv_nsec=99;
    timeout_ops.tv_sec=20;
    timeout_ops.tv_nsec=0xFF;

    ASSERT_EQ(params.getSSLCACheck(), false);
    params.setOperationTimeout(&timeout_co);
    ASSERT_EQ(params.getOperationTimeout()->tv_sec, 10);
    params.setConnectionTimeout(&timeout_ops);
    ASSERT_EQ(params.getConnectionTimeout()->tv_sec, 20);

    params.setTransparentRedirectionSupport(false);
    ASSERT_FALSE( params.getTransparentRedirectionSupport());

    Davix::RequestParams p2(&params);
    Davix::RequestParams p3(p2);

    ASSERT_EQ(p2.getOperationTimeout()->tv_sec, 10);
    ASSERT_EQ(p3.getOperationTimeout()->tv_sec, 10);

    ASSERT_EQ(p2.getConnectionTimeout()->tv_sec, 20);
    ASSERT_EQ(p3.getConnectionTimeout()->tv_sec, 20);

    ASSERT_FALSE( p2.getTransparentRedirectionSupport());

    Davix::RequestParams p4 = p3; // test deep copy

    ASSERT_EQ(p2.getOperationTimeout()->tv_sec, 10);
    ASSERT_EQ(p3.getOperationTimeout()->tv_sec, 10);

    ASSERT_EQ(p4.getConnectionTimeout()->tv_sec, 20);
 }

TEST(RequestParametersTest, CreateDeleteDyn){
    Davix::RequestParams* params = new Davix::RequestParams();

    ASSERT_EQ(params->getSSLCACheck(), true);
    ASSERT_EQ(params->getOperationTimeout()->tv_sec, DAVIX_DEFAULT_OPS_TIMEOUT);
    ASSERT_EQ(params->getConnectionTimeout()->tv_sec, DAVIX_DEFAULT_CONN_TIMEOUT);
    ASSERT_TRUE(params->getClientCertCallbackX509().second == NULL);
    ASSERT_TRUE(params->getClientCertCallbackX509().first ==  NULL);

    params->setSSLCAcheck(false);
    struct timespec timeout_co, timeout_ops;
    timeout_co.tv_sec =10;
    timeout_co.tv_nsec=99;
    timeout_ops.tv_sec=20;
    timeout_ops.tv_nsec=0xFF;

    ASSERT_EQ(params->getSSLCACheck(), false);
    params->setOperationTimeout(&timeout_co);
    ASSERT_EQ(params->getOperationTimeout()->tv_sec, 10);
    params->setConnectionTimeout(&timeout_ops);
    ASSERT_EQ(params->getConnectionTimeout()->tv_sec, 20);

    Davix::RequestParams *p2 = new Davix::RequestParams(params);
    Davix::RequestParams *p3 = new Davix::RequestParams(*p2);
    Davix::RequestParams p4(params);
    Davix::RequestParams p5(*p3);

    ASSERT_EQ(p2->getOperationTimeout()->tv_sec, 10);
    ASSERT_EQ(p3->getOperationTimeout()->tv_sec, 10);

    ASSERT_EQ(p2->getConnectionTimeout()->tv_sec, 20);
    ASSERT_EQ(p3->getConnectionTimeout()->tv_sec, 20);

    delete params;
    delete p2;
    delete p3;
 }


TEST(DavixErrorTest, CreateDelete){
    Davix::DavixError err("test_dav_scope", Davix::StatusCode::IsNotADirectory, " problem");
    ASSERT_EQ(err.getErrMsg(), " problem");
    ASSERT_EQ(err.getStatus(), Davix::StatusCode::IsNotADirectory);

    Davix::DavixError * err2=NULL;
    Davix::DavixError::setupError(&err2,"test_dav_scope2", Davix::StatusCode::ConnectionProblem, "connexion problem");
    ASSERT_EQ(err2->getErrMsg(), "connexion problem");
    ASSERT_EQ(err2->getStatus(), Davix::StatusCode::ConnectionProblem);

    Davix::DavixError::clearError(&err2);
    Davix::DavixError::clearError(&err2);
}


