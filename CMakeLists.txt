cmake_minimum_required(VERSION 3.15)
project(TryCatchMacros VERSION 0.1.2 LANGUAGES CXX)

option(TC_FORCE_NO_EXCEPTIONS "Force-build example with exceptions disabled (GCC/Clang)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tc_try_catch INTERFACE)
target_include_directories(tc_try_catch INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
add_library(tc::try_catch ALIAS tc_try_catch)

add_executable(example examples/main.cpp)
target_link_libraries(example PRIVATE tc_try_catch)

if (MSVC)
  # MSVC enables exceptions by default for C++ (/EHsc). We leave it as-is.
  target_compile_options(example PRIVATE /W4)
else()
  target_compile_options(example PRIVATE -Wall -Wextra -Wpedantic)
  if (TC_FORCE_NO_EXCEPTIONS)
    target_compile_options(example PRIVATE -fno-exceptions)
  endif()
endif()

include(CTest)
if (BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_executable(tc_tests
    tests/test_try_catch.cpp
    tests/test_throw_guard.cpp
    tests/test_logging.cpp
    tests/test_noexcept.cpp
  tests/test_catch_order_rethrow.cpp
  tests/test_helpers_abort.cpp
  tests/test_catch_do_as.cpp
  )
  target_link_libraries(tc_tests PRIVATE tc_try_catch GTest::gtest GTest::gtest_main)
  if (MSVC)
    target_compile_options(tc_tests PRIVATE /W4)
  else()
    target_compile_options(tc_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME tc_tests COMMAND tc_tests)

  add_executable(tc_tests_noex
    tests/test_try_catch.cpp
    tests/test_throw_guard.cpp
    tests/test_logging.cpp
    tests/test_noexcept.cpp
  tests/test_catch_order_rethrow.cpp
  tests/test_helpers_abort.cpp
  tests/test_catch_do_as.cpp
  )
  target_link_libraries(tc_tests_noex PRIVATE tc_try_catch GTest::gtest GTest::gtest_main)
  if (MSVC)
    target_compile_options(tc_tests_noex PRIVATE /W4)
  else()
    target_compile_options(tc_tests_noex PRIVATE -Wall -Wextra -Wpedantic -fno-exceptions)
  endif()
  add_test(NAME tc_tests_noex COMMAND tc_tests_noex)
endif()

# ---------- Install & package config (header-only interface) ----------
include(CMakePackageConfigHelpers)

set(TC_TARG_EXPORT_NAME tc_try_catchTargets)
set(TC_INSTALL_CONFIGDIR lib/cmake/tc_try_catch)

install(TARGETS tc_try_catch
    EXPORT ${TC_TARG_EXPORT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include)

install(EXPORT ${TC_TARG_EXPORT_NAME}
    NAMESPACE tc::
    FILE tc_try_catchTargets.cmake
    DESTINATION ${TC_INSTALL_CONFIGDIR})

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tc_try_catchConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tc_try_catchConfig.cmake
  INSTALL_DESTINATION ${TC_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/tc_try_catchConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/tc_try_catchConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/tc_try_catchConfigVersion.cmake
  DESTINATION ${TC_INSTALL_CONFIGDIR}
)
