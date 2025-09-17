#include <iostream>
#include <stdexcept>
#include "../include/tc/try_catch.hpp"

static int may_throw(int x) {
#if TC_EXCEPTIONS_ENABLED
    if (x < 0) {
        TC_THROW(std::runtime_error("x must be non-negative"));
    }
#else
    // In no-exception builds, signal failure via return code
    if (x < 0) {
        return -1;
    }
#endif
    return x * 2;
}

int main() {
    std::cout << "TC_EXCEPTIONS_ENABLED=" << TC_EXCEPTIONS_ENABLED
              << ", TC_DEBUG=" << TC_DEBUG << ", TC_RELEASE=" << TC_RELEASE << "\n";

    // Log at various levels
    TC_LOG_TRACE("trace message");
    TC_LOG_DEBUG("debug message");
    TC_LOG_INFO("info message");

    // Raise log level to warn at runtime
    tc::log::set_level(tc::log::level::warn);
    TC_LOG_INFO("this will likely be filtered");
    TC_LOG_WARN("warn visible");

    // Reset level to info
    tc::log::set_level(tc::log::level::info);

    int rc = 0;
    TC_TRY {
        int a = may_throw(5);
        std::cout << "ok: " << a << "\n";
        int b = may_throw(-1);
        std::cout << "should not reach: " << b << "\n";
    }
    TC_CATCH(const std::exception&, ex) {
        TC_ERROR("caught std::exception in example");
        rc = 1;
    }
    TC_CATCH_ALL() {
        TC_WARN("caught unknown exception in example");
        rc = 2;
    }

    bool ok = TC_GUARD(may_throw(1));
    std::cout << "TC_GUARD on valid input: " << (ok ? "true" : "false") << "\n";

#if !TC_EXCEPTIONS_ENABLED
    // Demonstrate alternative control flow in no-exception builds
    int c = may_throw(-2);
    if (c < 0) {
        std::cout << "no-exception error path taken (c=" << c << ")\n";
    }
#endif

    std::cout << "done\n";
    return rc;
}
