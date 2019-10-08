#include <gtest/gtest.h>
#include <status/DavixStatus.hpp>

using namespace Davix;

TEST(Status, Empty) {
  Status st;
  ASSERT_TRUE(st.ok());
  ASSERT_EQ(st.getErrorMessage(), "");
  ASSERT_EQ(st.getCode(), StatusCode::OK);
  ASSERT_EQ(st.getScope(), "");

  Status st2(st);
  ASSERT_TRUE(st2.ok());
  ASSERT_EQ(st2.getErrorMessage(), "");
  ASSERT_EQ(st2.getCode(), StatusCode::OK);
  ASSERT_EQ(st2.getScope(), "");

  Status st3 = st;
  ASSERT_TRUE(st3.ok());
  ASSERT_EQ(st3.getErrorMessage(), "");
  ASSERT_EQ(st3.getCode(), StatusCode::OK);
  ASSERT_EQ(st3.getScope(), "");
}

TEST(Status, Filled) {
  Status st("Davix::HttpRequest", StatusCode::IsNotADirectory, "An error message");
  ASSERT_FALSE(st.ok());
  ASSERT_EQ(st.getErrorMessage(), "An error message");
  ASSERT_EQ(st.getCode(), StatusCode::IsNotADirectory);
  ASSERT_EQ(st.getScope(), "Davix::HttpRequest");

  Status st2(st);
  ASSERT_FALSE(st2.ok());
  ASSERT_EQ(st2.getErrorMessage(), "An error message");
  ASSERT_EQ(st2.getCode(), StatusCode::IsNotADirectory);
  ASSERT_EQ(st2.getScope(), "Davix::HttpRequest");

  Status st3 = st;
  ASSERT_FALSE(st3.ok());
  ASSERT_EQ(st3.getErrorMessage(), "An error message");
  ASSERT_EQ(st3.getCode(), StatusCode::IsNotADirectory);
  ASSERT_EQ(st3.getScope(), "Davix::HttpRequest");
}
