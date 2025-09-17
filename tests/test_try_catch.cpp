#include <gtest/gtest.h>
#include "../include/tc/try_catch.hpp"
#include <stdexcept>

#if TC_EXCEPTIONS_ENABLED
TEST(TryCatch, BasicCatchStdException) {
    int step = 0;
    TC_TRY {
        step = 1;
        TC_THROW(std::runtime_error("boom"));
        step = 2;
    } TC_CATCH(const std::exception&, ex) {
        (void)ex;
        step = 3;
    } TC_CATCH_ALL() {
        step = 4;
    }
    EXPECT_EQ(step, 3);
}

TEST(TryCatch, CatchAll) {
    int caught = 0;
    TC_TRY {
        throw 42; // non-std exception
    } TC_CATCH(const std::exception&, e) {
        (void)e;
    } TC_CATCH_ALL() {
        caught = 1;
    }
    EXPECT_EQ(caught, 1);
}
#else
TEST(TryCatchNoEx, TryRunsCatchSkipped) {
    int try_ran = 0;
    int catch_ran = 0;
    TC_TRY {
        try_ran = 1;
    } TC_CATCH(const std::exception&, unused) {
        catch_ran = 1;
    } TC_CATCH_ALL() {
        catch_ran = 1;
    }
    EXPECT_EQ(try_ran, 1);
    EXPECT_EQ(catch_ran, 0);
}
#endif
