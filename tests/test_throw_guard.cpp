#include <gtest/gtest.h>
#include "../include/tc/try_catch.hpp"
#include <stdexcept>

static int may_throw(int x) {
#if TC_EXCEPTIONS_ENABLED
    if (x < 0) TC_THROW(std::runtime_error("neg"));
    return x;
#else
    // No exceptions: no throw
    return x;
#endif
}

TEST(Guard, ReturnsFalseOnExceptionWhenEnabled) {
#if TC_EXCEPTIONS_ENABLED
    EXPECT_TRUE(TC_GUARD(may_throw(1)));
    EXPECT_FALSE(TC_GUARD(may_throw(-1)));
#else
    // In no-exception builds, TC_GUARD always returns true
    EXPECT_TRUE(TC_GUARD(may_throw(1)));
    EXPECT_TRUE(TC_GUARD(may_throw(-1)));
#endif
}
