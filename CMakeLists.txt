# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)


cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/cmake-common-utils")
include(PreferClang)
include(PreferCcache)
include(DefaultDebug)
include(PreferGoldLinker)

project(fservice VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

# Add possibility to sanitize code
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/tools/cmake/sanitizers-cmake/cmake/")
find_package(Sanitizers REQUIRED)

# Add possibility to get code coverage
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/CMake-codecov/cmake")
find_package(codecov REQUIRED)

# Set global warnings configuration for all sub-projects
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Common deps
# Boost deps
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
# Boost is configured via Folly
# find_package(Boost REQUIRED COMPONENTS program_options)
# Other deps
find_package(fmt REQUIRED)
find_package(folly REQUIRED)
find_package(log4cplus REQUIRED)
find_package(nng REQUIRED)
find_package(nngpp REQUIRED)

# Lib configuration

set(LIB_NAME FServiceLib)
set(LIB_SRC_LIST
    "fservice/GeneralError.h"
    "fservice/GeneralError.cpp"
    "fservice/Engine.h"
    "fservice/Engine.cpp"
    "fservice/EngineLauncher.h"
    "fservice/EngineLauncher.cpp"
    "fservice/StartupConfig.h"
    "fservice/StartupConfig.cpp"
    "fservice/Version.h"
    "fservice/Version.cpp"
    "fservice/Logger.h"
    "fservice/Logger.cpp"
    "fservice/Outcome.h"
    "fservice/PathUtil.h"
    "fservice/PathUtil.cpp"
    "fservice/ScopeGuard.h"
    "fservice/SignalHandler.h"
    "fservice/SignalHandler.cpp"
    "fservice/RepeatableTimeout.h"
    )

add_library(${LIB_NAME} ${LIB_SRC_LIST})
add_library(fservice::${LIB_NAME} ALIAS ${LIB_NAME})
# Workaround: if library has no cpp files the following line should be added
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
# make possible to include headers using "<project/file>" notation
target_include_directories(${LIB_NAME}
  BEFORE
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_sanitizers(${LIB_NAME})
add_coverage(${LIB_NAME} fservice)
target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)
target_link_libraries(${LIB_NAME} PUBLIC fmt::fmt log4cplus::log4cplusS Folly::folly nng::nng nng::nngpp)

# App configuration
set(APP_NAME fservice)
add_executable(${APP_NAME} "fservice/LifeCycle.cpp")
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)
add_sanitizers(${APP_NAME})
list(APPEND LCOV_REMOVE_PATTERNS "'*fservice/LifeCycle.cpp'")
target_link_libraries(${APP_NAME} PRIVATE ${LIB_NAME})

# Copy default config to the output dir
configure_file(config/logger.cfg logger.cfg COPYONLY)
configure_file(config/${CMAKE_PROJECT_NAME}.cfg ${CMAKE_PROJECT_NAME}.cfg COPYONLY)

if (BUILD_TESTING)
    find_package(Catch2 REQUIRED)

    set(TEST_LIB_NAME "${LIB_NAME}Test")

    set(TEST_SRC_LIST
        "fservice/tests/EnumUtilTest.cpp"
        "fservice/tests/PathUtilTest.cpp"
        "fservice/tests/ScopeGuardTest.cpp"
        "fservice/tests/EngineTest.cpp"
        )

    add_library(${TEST_LIB_NAME} OBJECT ${TEST_SRC_LIST})
    add_library(fservice::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

    add_sanitizers(${TEST_LIB_NAME})
    list(APPEND LCOV_REMOVE_PATTERNS "'*/tests/*'")
    add_coverage(${TEST_LIB_NAME})

    target_include_directories(${TEST_LIB_NAME} PRIVATE tests)
    target_compile_features(${TEST_LIB_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TEST_LIB_NAME} PUBLIC fservice::${LIB_NAME} Catch2::Catch2)

    # define test runner
    set(TEST_RUNNER_NAME testrunner)

    add_executable(${TEST_RUNNER_NAME} "fservice/tests/TestRunner.cpp")

    target_compile_features(${TEST_RUNNER_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TEST_RUNNER_NAME} PRIVATE ${TEST_LIB_NAME})

    add_sanitizers(${TEST_RUNNER_NAME})

    add_test(NAME all COMMAND ${TEST_RUNNER_NAME})
endif()

include(ClangTidy)
include(PrepareDoxygen)
include(ClangStaticAnalyzer)