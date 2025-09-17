#include <gtest/gtest.h>
#include "../include/tc/try_catch.hpp"
#include <stdexcept>

#if TC_EXCEPTIONS_ENABLED
TEST(Helpers, StdCatchWarnErrorMacrosCompile) {
    // Compile-only: ensure helper macros parse in a valid try/catch sequence
    TC_TRY { TC_THROW(std::runtime_error("x")); }
    TC_CATCH_STD_WARN()
    TC_CATCH_ALL_ERROR()
    SUCCEED();
}
#else
// In no-exception builds, TC_THROW leads to TC_ABORT by default; we won't call it in tests.
// Instead just ensure macros expand and compile, and catch blocks are compiled out.
TEST(HelpersNoEx, CatchMacrosCompileAway) {
    int ran = 0;
    TC_TRY { ran = 1; }
    TC_CATCH_STD_WARN()
    TC_CATCH_ALL_ERROR()
    EXPECT_EQ(ran, 1);
}
#endif
