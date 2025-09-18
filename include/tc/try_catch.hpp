// tc/try_catch.hpp
// Lightweight macro layer to manage try/catch across build types and exception settings
// - Cross-platform: GCC/Clang/MSVC
// - Works when exceptions are disabled (e.g., -fno-exceptions) by compiling catch paths out
// - Provides optimization hints and convenience helpers
//
// Usage pattern:
//   TC_TRY {
//     // code that may throw
//   } TC_CATCH(const std::exception& e) {
//     // handle
//   } TC_CATCH_ALL() {
//     // fallback
//   }
//
// Additional helpers:
//   - TC_EXCEPTIONS_ENABLED (0/1)
//   - TC_DEBUG / TC_RELEASE
//   - TC_LIKELY(x) / TC_UNLIKELY(x)
//   - TC_NOEXCEPT_IF_NOEXCEPTIONS (adds noexcept when exceptions are disabled)
//   - TC_THROW(expr) and TC_RETHROW(): safe in no-exception builds (abort by default)
//   - TC_ABORT(msg): abort helper used by TC_THROW in no-exception builds
//
// You may customize behaviors by defining before including this header:
//   - TC_ON_NOEXCEPT_THROW(file,line,func,msg): user-defined hook instead of abort
//   - TC_ENABLE_LOGGING (0/1): default 1 in Debug, 0 in Release
//
// This file is header-only and has no external dependencies.

#pragma once

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <exception>

// ===================== Build-type detection =====================
#if !defined(TC_DEBUG) && !defined(TC_RELEASE)
#if defined(NDEBUG)
#define TC_RELEASE 1
#define TC_DEBUG 0
#else
#define TC_RELEASE 0
#define TC_DEBUG 1
#endif
#endif

// ===================== Exception support detection =====================
// GCC/Clang define __EXCEPTIONS when -fexceptions is on. C++11 and later may define __cpp_exceptions.
// MSVC defines _CPPUNWIND when C++ exceptions are enabled; _HAS_EXCEPTIONS can be used by the STL.
#if !defined(TC_EXCEPTIONS_ENABLED)
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
#define TC_EXCEPTIONS_ENABLED 1
#elif defined(_MSC_VER) && defined(_CPPUNWIND)
#define TC_EXCEPTIONS_ENABLED 1
#elif defined(_HAS_EXCEPTIONS) && (_HAS_EXCEPTIONS == 1)
#define TC_EXCEPTIONS_ENABLED 1
#else
#define TC_EXCEPTIONS_ENABLED 0
#endif
#endif

// ===================== Logging control =====================
#if !defined(TC_ENABLE_LOGGING)
#if TC_DEBUG
#define TC_ENABLE_LOGGING 1
#else
#define TC_ENABLE_LOGGING 0
#endif
#endif

namespace tc {
namespace detail {

inline void default_abort_noexcept(const char* file, int line, const char* func, const char* msg) {
    std::fprintf(stderr, "[tc] fatal: exception thrown but exceptions are disabled\n  at %s:%d in %s\n  msg: %s\n",
                 file ? file : "(unknown)", line, func ? func : "(unknown)", msg ? msg : "(none)");
    std::fflush(stderr);
    std::abort();
}

enum class log_level : int { trace = 0, debug = 1, info = 2, warn = 3, error = 4, off = 5 };

using log_sink_t = void (*)(log_level, const char* file, int line, const char* func, const char* fmt, va_list ap);

inline void default_stderr_sink(log_level lvl, const char* file, int line, const char* func, const char* fmt,
                                va_list ap) {
    const char* name = "LOG";
    switch (lvl) {
    case log_level::trace:
        name = "TRACE";
        break;
    case log_level::debug:
        name = "DEBUG";
        break;
    case log_level::info:
        name = "INFO";
        break;
    case log_level::warn:
        name = "WARN";
        break;
    case log_level::error:
        name = "ERROR";
        break;
    case log_level::off:
        name = "OFF";
        break;
    }
    std::fprintf(stderr, "[%s] %s:%d %s: ", name, file ? file : "(unknown)", line, func ? func : "(unknown)");
    std::vfprintf(stderr, fmt ? fmt : "(null)", ap);
    std::fputc('\n', stderr);
}

inline std::atomic<int>& runtime_log_level() {
    auto initial = []() constexpr -> int {
#if TC_DEBUG
    return static_cast<int>(log_level::debug);
#else
    return static_cast<int>(log_level::info);
#endif
    }();
    static std::atomic<int> lvl{initial};
    return lvl;
}

inline std::atomic<log_sink_t>& runtime_sink() {
    static std::atomic<log_sink_t> sink{&default_stderr_sink};
    return sink;
}

inline void set_log_level(log_level lvl) {
    runtime_log_level().store(static_cast<int>(lvl), std::memory_order_relaxed);
}

inline log_level get_log_level() {
    return static_cast<log_level>(runtime_log_level().load(std::memory_order_relaxed));
}

inline void set_log_sink(log_sink_t sink) {
    runtime_sink().store(sink, std::memory_order_relaxed);
}

inline log_sink_t get_log_sink() {
    return runtime_sink().load(std::memory_order_relaxed);
}

inline void vlog_dispatch(log_level lvl, const char* file, int line, const char* func, const char* fmt, va_list ap) {
    if (static_cast<int>(lvl) < runtime_log_level().load(std::memory_order_relaxed))
        return;
    auto* s = runtime_sink().load(std::memory_order_relaxed);
    if (s)
        s(lvl, file, line, func, fmt, ap);
}

inline void logf(log_level lvl, const char* file, int line, const char* func, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_dispatch(lvl, file, line, func, fmt, ap);
    va_end(ap);
}

} // namespace detail
} // namespace tc

// ===================== Branch prediction hints =====================
#if defined(__GNUC__) || defined(__clang__)
#define TC_LIKELY(x) (__builtin_expect(!!(x), 1))
#define TC_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define TC_LIKELY(x) (x)
#define TC_UNLIKELY(x) (x)
#endif

// ===================== noexcept helpers =====================
#if TC_EXCEPTIONS_ENABLED
#define TC_NOEXCEPT_IF_NOEXCEPTIONS /* nothing */
#else
#define TC_NOEXCEPT_IF_NOEXCEPTIONS noexcept
#endif

// ===================== Abort and throw helpers =====================
#if !defined(TC_ABORT)
#define TC_ABORT(msg) ::tc::detail::default_abort_noexcept(__FILE__, __LINE__, __func__, (msg))
#endif

#if TC_EXCEPTIONS_ENABLED
#define TC_THROW(ex) throw(ex)
#define TC_RETHROW() throw
#else
// When exceptions are disabled, throwing is a fatal error by default.
#define TC_THROW(ex) TC_ABORT("TC_THROW called with exceptions disabled")
#define TC_RETHROW() TC_ABORT("TC_RETHROW called with exceptions disabled")
#endif

// ===================== try/catch macros =====================
// These macros allow source to remain uniform across exception-enabled/disabled builds.
// When exceptions are disabled, TC_TRY becomes if(true) and TC_CATCH* become else if(false),
// effectively compiling out the catch handlers while preserving syntax and scopes.

#if TC_EXCEPTIONS_ENABLED

#define TC_TRY try
#define TC_CATCH(T, n) catch (T n)
#define TC_CATCH_ALL() catch (...)

#else

#define TC_TRY if (true)
#define TC_CATCH(T, n) else if (false)
#define TC_CATCH_ALL() else if (false)

#endif

// ===================== Logging macros (leveled + compatibility) =====================
#if !defined(TC_ENABLE_ERROR_LOGGING)
#define TC_ENABLE_ERROR_LOGGING 1
#endif

// Level macros. Note: avoid name clash with TC_DEBUG macro.
#define TC_LOG_TRACE(...) ::tc::detail::logf(::tc::detail::log_level::trace, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define TC_LOG_DEBUG(...) ::tc::detail::logf(::tc::detail::log_level::debug, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define TC_LOG_INFO(...) ::tc::detail::logf(::tc::detail::log_level::info, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define TC_LOG_WARN(...) ::tc::detail::logf(::tc::detail::log_level::warn, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define TC_LOG_ERROR(...) ::tc::detail::logf(::tc::detail::log_level::error, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Compatibility aliases
#if TC_ENABLE_LOGGING
#define TC_WARN(...) TC_LOG_WARN(__VA_ARGS__)
#else
#define TC_WARN(...) ((void)0)
#endif

#if TC_ENABLE_ERROR_LOGGING
#define TC_ERROR(...) TC_LOG_ERROR(__VA_ARGS__)
#else
#define TC_ERROR(...) ((void)0)
#endif

// Runtime log control API (header-only; inline via namespace tc::detail)
namespace tc {
namespace log {
using level = ::tc::detail::log_level;
inline void set_level(level v) {
    ::tc::detail::set_log_level(v);
}
inline level get_level() {
    return ::tc::detail::get_log_level();
}
using sink_t = ::tc::detail::log_sink_t;
inline void set_sink(sink_t s) {
    ::tc::detail::set_log_sink(s);
}
inline sink_t get_sink() {
    return ::tc::detail::get_log_sink();
}
} // namespace log
} // namespace tc

// ===================== Convenience wrap macros (optional) =====================
// TC_GUARD(expr): Run expr inside TC_TRY and convert any exception to a boolean failure.
// Returns true if ran without exception; false if caught. In no-exception builds it's always true.
#define TC_GUARD(expr)                                                                                                 \
    ([&]() TC_NOEXCEPT_IF_NOEXCEPTIONS -> bool {                                                                       \
        bool ok = true;                                                                                                \
        TC_TRY {                                                                                                       \
            (void)(expr);                                                                                              \
        }                                                                                                              \
        TC_CATCH(const std::exception&, _tc_unused) {                                                                  \
            ok = false;                                                                                                \
        }                                                                                                              \
        TC_CATCH_ALL() {                                                                                               \
            ok = false;                                                                                                \
        }                                                                                                              \
        return ok;                                                                                                     \
    }())

// Ready-made catch helpers to pair with try/catch and emit warnings/errors.
#if TC_EXCEPTIONS_ENABLED
#define TC_CATCH_STD_WARN()                                                                                            \
    TC_CATCH(const std::exception&, _tc_e) {                                                                           \
        TC_WARN("exception: %s", _tc_e.what());                                                                        \
    }
#define TC_CATCH_STD_ERROR()                                                                                           \
    TC_CATCH(const std::exception&, _tc_e) {                                                                           \
        TC_ERROR("exception: %s", _tc_e.what());                                                                       \
    }
#else
#define TC_CATCH_STD_WARN()                                                                                            \
    TC_CATCH(const std::exception&, _tc_unused) {                                                                      \
        TC_WARN("exception handler (no-exceptions build)");                                                            \
    }
#define TC_CATCH_STD_ERROR()                                                                                           \
    TC_CATCH(const std::exception&, _tc_unused) {                                                                      \
        TC_ERROR("exception handler (no-exceptions build)");                                                           \
    }
#endif

#define TC_CATCH_ALL_WARN()                                                                                            \
    TC_CATCH_ALL() {                                                                                                   \
        TC_WARN("unknown exception");                                                                                  \
    }
#define TC_CATCH_ALL_ERROR()                                                                                           \
    TC_CATCH_ALL() {                                                                                                   \
        TC_ERROR("unknown exception");                                                                                 \
    }

// Variants that allow custom user body to run inside the catch block.
// Usage example:
//   TC_TRY { ... }
//   TC_CATCH_STD_WARN_DO({ metric++; })
//   TC_CATCH_ALL_ERROR_DO({ cleanup(); })
// Or with a named exception variable:
//   TC_CATCH_STD_WARN_AS(e, { TC_LOG_INFO("%s", e.what()); })
#if TC_EXCEPTIONS_ENABLED
#define TC_CATCH_STD_WARN_DO(BODY)                                                                                     \
    TC_CATCH(const std::exception&, _tc_e) {                                                                           \
        TC_WARN("exception: %s", _tc_e.what());                                                                        \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_ERROR_DO(BODY)                                                                                    \
    TC_CATCH(const std::exception&, _tc_e) {                                                                           \
        TC_ERROR("exception: %s", _tc_e.what());                                                                       \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_WARN_AS(NAME, BODY)                                                                               \
    TC_CATCH(const std::exception&, NAME) {                                                                            \
        TC_WARN("exception: %s", NAME.what());                                                                         \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_ERROR_AS(NAME, BODY)                                                                              \
    TC_CATCH(const std::exception&, NAME) {                                                                            \
        TC_ERROR("exception: %s", NAME.what());                                                                        \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#else
#define TC_CATCH_STD_WARN_DO(BODY)                                                                                     \
    TC_CATCH(const std::exception&, _tc_unused) {                                                                      \
        TC_WARN("exception handler (no-exceptions build)");                                                            \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_ERROR_DO(BODY)                                                                                    \
    TC_CATCH(const std::exception&, _tc_unused) {                                                                      \
        TC_ERROR("exception handler (no-exceptions build)");                                                           \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_WARN_AS(NAME, BODY)                                                                               \
    TC_CATCH(const std::exception&, NAME) {                                                                            \
        const std::exception& NAME = *static_cast<const std::exception*>(nullptr);                                     \
        TC_WARN("exception handler (no-exceptions build)");                                                            \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_STD_ERROR_AS(NAME, BODY)                                                                              \
    TC_CATCH(const std::exception&, NAME) {                                                                            \
        const std::exception& NAME = *static_cast<const std::exception*>(nullptr);                                     \
        TC_ERROR("exception handler (no-exceptions build)");                                                           \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#endif

#define TC_CATCH_ALL_WARN_DO(BODY)                                                                                     \
    TC_CATCH_ALL() {                                                                                                   \
        TC_WARN("unknown exception");                                                                                  \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }
#define TC_CATCH_ALL_ERROR_DO(BODY)                                                                                    \
    TC_CATCH_ALL() {                                                                                                   \
        TC_ERROR("unknown exception");                                                                                 \
        do {                                                                                                           \
            BODY;                                                                                                      \
        } while (0);                                                                                                   \
    }

// ===================== Versioning =====================
#define TC_TRY_CATCH_VERSION_MAJOR 0
#define TC_TRY_CATCH_VERSION_MINOR 1
#define TC_TRY_CATCH_VERSION_PATCH 0
