#include <gtest/gtest.h>
#include "../include/tc/try_catch.hpp"

struct X {
    int f() TC_NOEXCEPT_IF_NOEXCEPTIONS {
        return 42;
    }
};

TEST(Noexcept, ConditionalNoexcept) {
    X x;
#if TC_EXCEPTIONS_ENABLED
    EXPECT_FALSE(noexcept(x.f()));
#else
    EXPECT_TRUE(noexcept(x.f()));
#endif
}

TEST(Noexcept, LikelyUnlikely) {
    int v = 1;
    // Just ensure they compile and evaluate
    EXPECT_EQ(TC_LIKELY(v), 1);
    EXPECT_EQ(TC_UNLIKELY(v), 1);
}
