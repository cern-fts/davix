

#include <davixcontext.h>
#include <posixgate.h>
#include <gtest/gtest.h>


// instanciate and play with gates
TEST(ContextTest, CreateDelete){
    Davix::Context c1;

    Davix::Context* c2 = c1.clone();
    ASSERT_TRUE(c2 != NULL);

    Davix::PosixGate* p1 = c1.posixGate();
    ASSERT_TRUE(p1 != NULL);

    Davix::PosixGate* p2 = c2->posixGate();
    ASSERT_TRUE(p2 != NULL);
    delete p2;
    delete p1;
    p2 = c1.posixGate();
    ASSERT_TRUE(p2 != NULL);

    delete c2;

}


