#include <gtest/gtest.h>
#include "../include/tc/try_catch.hpp"
#include <stdexcept>

#if TC_EXCEPTIONS_ENABLED
TEST(CatchDoAs, StdWarnDoAndAs) {
    int n = 0;
    int m = 0;
    TC_TRY { throw std::runtime_error("boom"); }
    TC_CATCH_STD_WARN_DO({ n++; })
    // std::exception-based catch should handle the runtime_error and run BODY once.
    EXPECT_EQ(n, 1);
    EXPECT_EQ(m, 0);
}

TEST(CatchDoAs, AllErrorDo) {
    int x = 0;
    TC_TRY { throw 1; }
    TC_CATCH_ALL_ERROR_DO({ x = 42; })
    EXPECT_EQ(x, 42);
}
#else
TEST(CatchDoAsNoEx, CompileAwayButBodyPresent) {
    // In no-exception builds, the TC_CATCH* become else-if(false) blocks, body code is compiled but not executed.
    int n = 0;
    int m = 0;
    TC_TRY { n = 1; }
    TC_CATCH_STD_WARN_DO({ n = 99; })
    TC_CATCH_STD_WARN_AS(e, { (void)e; m = 2; })
    TC_CATCH_ALL_ERROR_DO({ m = 3; })
    EXPECT_EQ(n, 1);
    EXPECT_EQ(m, 0);
}
#endif
