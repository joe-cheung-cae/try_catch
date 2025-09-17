# Contributing

Thanks for your interest! A few quick notes:

- Use C++17; keep the library header-only and minimal.
- Run tests with both exception modes:
  - Exceptions on: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build && ctest --test-dir build -V`
  - Exceptions off: `cmake --build build --target tc_tests_noex && ./build/tc_tests_noex`
- Keep macros portable across GCC/Clang/MSVC.
- Prefer small, focused PRs with clear titles.

## Coding style

- Use `.clang-format` in repo root.
- Avoid unnecessary dependencies.

## Commit messages

- Imperative mood; e.g., `CI: fix Windows parallel flag`.

## License

By contributing, you agree your contributions are licensed under the MIT License.
