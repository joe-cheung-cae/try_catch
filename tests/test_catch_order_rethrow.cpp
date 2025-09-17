#include "../include/tc/try_catch.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

#if TC_EXCEPTIONS_ENABLED
TEST(CatchOrder, SpecificBeforeGeneral) {
    int which = 0;
    TC_TRY {
        TC_THROW(std::runtime_error("rte"));
    }
    TC_CATCH(const std::runtime_error&, e1) {
        (void)e1;
        which = 1;
    }
    TC_CATCH(const std::exception&, e2) {
        (void)e2;
        which = 2;
    }
    TC_CATCH_ALL() {
        which = 3;
    }
    EXPECT_EQ(which, 1);
}

TEST(Rethrow, InnerCatchRethrowOuterHandles) {
    int inner = 0, outer = 0;
    TC_TRY {
        TC_TRY {
            TC_THROW(std::runtime_error("rte"));
        }
        TC_CATCH(const std::runtime_error&, e) {
            (void)e;
            inner = 1;
            TC_RETHROW();
        }
    }
    TC_CATCH(const std::exception&, e2) {
        (void)e2;
        outer = 1;
    }
    EXPECT_EQ(inner, 1);
    EXPECT_EQ(outer, 1);
}
#else
TEST(CatchOrderNoEx, CompilesButNoRuntime) {
    SUCCEED();
}
#endif
