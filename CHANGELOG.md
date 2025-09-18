# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.2] - 2025-09-18
### Added
- Optional manual `format-check` GitHub Actions job (disabled by default; trigger via workflow_dispatch with `run_format=true`).

### Changed
- Bump library patch version macros to 0.1.2.

### Removed
- Automatic `clang-format` lint step from default CI pipeline (now manual only).

## [0.1.1] - 2025-09-??
### Fixed
- Release workflow permissions (403) by adding appropriate token permissions and full fetch depth.
- Formatting false positives around atomic initialization via refactor.

### Added
- Pre-commit hook for automatic clang-format on staged files.

## [0.1.0] - 2025-09-??
### Added
- Initial release: header-only try/catch abstraction macros, logging system, guard macro, helper catch macros (WARN/ERROR, DO/AS variants).
- Comprehensive GoogleTest suite (exceptions on/off builds).
- CMake interface target with install/export + package config.
- Multi-platform CI build/test and release workflow.

[0.1.2]: https://github.com/joe-cheung-cae/try_catch/releases/tag/v0.1.2
[0.1.1]: https://github.com/joe-cheung-cae/try_catch/releases/tag/v0.1.1
[0.1.0]: https://github.com/joe-cheung-cae/try_catch/releases/tag/v0.1.0
