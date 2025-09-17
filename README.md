# Try/Catch Macro Layer

Header-only macros to manage C++ try/catch across platforms, build types, and exception settings.

## Why

- Uniform source that compiles with or without C++ exceptions
- Easy to toggle behavior by build type (Debug/Release)
- Small, dependency-free, cross-platform (GCC/Clang/MSVC)

## Install

Just add `include/` to your include paths and include `tc/try_catch.hpp`.

With CMake in this repo:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -v
./build/example

# Force-disable exceptions on GCC/Clang
cmake -S . -B build-noex -DTC_FORCE_NO_EXCEPTIONS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-noex -v
./build-noex/example
```

## Macros

- `TC_TRY`, `TC_CATCH(T, name)`, `TC_CATCH_ALL()`
- `TC_THROW(ex)`, `TC_RETHROW()`
- `TC_EXCEPTIONS_ENABLED` (0/1)
- `TC_DEBUG`, `TC_RELEASE`
- `TC_LIKELY(x)`, `TC_UNLIKELY(x)`
- `TC_NOEXCEPT_IF_NOEXCEPTIONS`
- `TC_GUARD(expr)` -> bool

Behavior when exceptions are disabled (`-fno-exceptions` or equivalent):

- `TC_TRY { ... } TC_CATCH(...) { ... }` compiles to an `if(true){...} else if(false){...}` pattern; catch blocks are not compiled.
- `TC_THROW` and `TC_RETHROW` call `TC_ABORT()` by default. Override via `#define TC_ABORT(msg) ...` to customize.

## Example

See `examples/main.cpp`.

## Customize

- Define `TC_ABORT(msg)` before including the header to customize fatal handler when exceptions are disabled.
- Define `TC_ENABLE_LOGGING` to 0/1 as needed.

## Notes

- MSVC: exceptions are controlled by `/EHsc`. This sample doesn't force-disable exceptions on MSVC.
- GCC/Clang: use `-fno-exceptions` to disable. CMake option `TC_FORCE_NO_EXCEPTIONS` applies only to the example target.
