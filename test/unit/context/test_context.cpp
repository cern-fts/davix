

#include <davixcontext.h>
#include <posixgate.h>
#include <gtest/gtest.h>


// instanciate and play with gates
TEST(ContextTest, CreateDelete){
    Davix::Context c1;

    Davix::Context* c2 = c1.clone();
    ASSERT_TRUE(c2 != NULL);

    Davix::PosixGate p1 = c1.posixGate();

    Davix::PosixGate p2 = c2->posixGate();
    p2 = c1.posixGate();

    delete c2;

}




TEST(ContextTest, TestConfig){
    Davix::Context* c1 = new Davix::Context();
    struct timespec t1, t2, t3;
    t1.tv_sec = 1;
    t2.tv_sec = ((time_t) 0) -1;
    t2.tv_nsec = 0xFF;
    t3.tv_sec = 3;

    ASSERT_TRUE(c1->getSSLCACheck());
    ASSERT_EQ(DAVIX_DEFAULT_CONN_TIMEOUT, c1->getConnexionTimeout()->tv_sec);
    ASSERT_EQ(DAVIX_DEFAULT_OPS_TIMEOUT, c1->getOperationTimeout()->tv_sec);

    c1->setConnexionTimeout(&t1);
    c1->setOperationTimeout(&t2);
    c1->setSSLCACheck(false);

    ASSERT_TRUE(c1->getSSLCACheck() == false);

    ASSERT_EQ(t1.tv_sec, c1->getConnexionTimeout()->tv_sec);
    ASSERT_EQ(t2.tv_sec, c1->getOperationTimeout()->tv_sec);

    Davix::Context* c2 = c1->clone();
    ASSERT_EQ(t1.tv_sec, c2->getConnexionTimeout()->tv_sec);

    c2->setConnexionTimeout(&t3);
    ASSERT_EQ(t1.tv_sec, c1->getConnexionTimeout()->tv_sec);
    ASSERT_EQ(t3.tv_sec, c2->getConnexionTimeout()->tv_sec);
    c1->setConnexionTimeout(&t2);
    ASSERT_EQ(t2.tv_sec, c1->getConnexionTimeout()->tv_sec);
    ASSERT_EQ(t2.tv_nsec, c1->getConnexionTimeout()->tv_nsec);
    ASSERT_EQ(t3.tv_sec, c2->getConnexionTimeout()->tv_sec);

    delete c1;
    delete c2;

}


