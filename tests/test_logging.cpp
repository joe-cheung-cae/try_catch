#include "../include/tc/try_catch.hpp"
#include <cstdarg>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {
struct MemSink {
    static std::vector<std::string>& lines() {
        static std::vector<std::string> v;
        return v;
    }
    static void sink(::tc::detail::log_level lvl, const char* file, int line, const char* func, const char* fmt,
                     va_list ap) {
        (void)file;
        (void)line;
        (void)func;
        char buf[256];
        vsnprintf(buf, sizeof(buf), fmt, ap);
        std::string level;
        switch (lvl) {
        case ::tc::detail::log_level::trace:
            level = "TRACE";
            break;
        case ::tc::detail::log_level::debug:
            level = "DEBUG";
            break;
        case ::tc::detail::log_level::info:
            level = "INFO";
            break;
        case ::tc::detail::log_level::warn:
            level = "WARN";
            break;
        case ::tc::detail::log_level::error:
            level = "ERROR";
            break;
        default:
            level = "?";
            break;
        }
        lines().push_back(level + ":" + buf);
    }
};
} // namespace

TEST(Logging, LevelFilterAndSink) {
    auto prev_sink = ::tc::detail::get_log_sink();
    auto prev_lvl = ::tc::detail::get_log_level();
    ::tc::detail::set_log_sink(&MemSink::sink);
    ::tc::detail::set_log_level(::tc::detail::log_level::info);

    MemSink::lines().clear();
    TC_LOG_DEBUG("hidden");
    TC_LOG_INFO("show %d", 1);
    TC_LOG_WARN("warn %s", "x");
    TC_LOG_ERROR("err");

    ASSERT_EQ(MemSink::lines().size(), 3u);
    EXPECT_EQ(MemSink::lines()[0], std::string("INFO:show 1"));
    EXPECT_EQ(MemSink::lines()[1], std::string("WARN:warn x"));
    EXPECT_EQ(MemSink::lines()[2], std::string("ERROR:err"));

    ::tc::detail::set_log_sink(prev_sink);
    ::tc::detail::set_log_level(prev_lvl);
}
