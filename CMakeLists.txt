# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)


cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/cmake-common-utils")
#include(PreferClang)
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
find_package(fmt CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(log4cplus CONFIG REQUIRED)

# Setup protobuf ang gRPC
# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
#set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
find_program(PROTOBUF_PROTOC protoc)
message(STATUS "PROTOC: ${PROTOBUF_PROTOC}")

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(GRPC_GRPCPP gRPC::grpc++)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
message(STATUS "GRPC_CPP_PLUGIN_EXECUTABLE: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# Proto file
get_filename_component(SERVICE_PROTO "protos/Greeter.proto" ABSOLUTE)
get_filename_component(SERVICE_PROTO_PATH "${SERVICE_PROTO}" PATH)
message(STATUS "SERVICE_PROTO_PATH = ${SERVICE_PROTO_PATH}")

set(SERVICE_PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/protos")
#Force creation of the output dir. If generator is make and output dir
#doesn't exist then build will fail due to missing dir.
file(MAKE_DIRECTORY ${SERVICE_PROTO_GEN_DIR})

# Generated sources
set(SERVICE_PROTO_SRCS "${SERVICE_PROTO_GEN_DIR}/Greeter.pb.cc")
set(SERVICE_PROTO_HDRS "${SERVICE_PROTO_GEN_DIR}/Greeter.pb.h")
set(SERVICE_PROTO_GRPC_SRCS "${SERVICE_PROTO_GEN_DIR}/Greeter.grpc.pb.cc")
set(SERVICE_PROTO_RGPC_HDRS "${SERVICE_PROTO_GEN_DIR}/Greeter.grpc.pb.h")
message(STATUS "SERVICE_PROTO_SRCS = ${SERVICE_PROTO_SRCS}")

add_custom_command(
      OUTPUT "${SERVICE_PROTO_SRCS}" "${SERVICE_PROTO_HDRS}" "${SERVICE_PROTO_GRPC_SRCS}" "${SERVICE_PROTO_RGPC_HDRS}"
      COMMAND ${PROTOBUF_PROTOC}
      ARGS --grpc_out "${SERVICE_PROTO_GEN_DIR}"
        --cpp_out "${SERVICE_PROTO_GEN_DIR}"
        -I "${SERVICE_PROTO_PATH}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${SERVICE_PROTO}"
      DEPENDS "${SERVICE_PROTO}")

set(PROTOS_LIB_NAME FServiceProtosLib)
set(PROTOS_LIB_SRC_LIST
  ${SERVICE_PROTO_SRCS}
  ${SERVICE_PROTO_HDRS}
  ${SERVICE_PROTO_GRPC_SRCS}
  ${SERVICE_PROTO_GRPC_HDRS}
)

add_library(${PROTOS_LIB_NAME} ${PROTOS_LIB_SRC_LIST})
add_library(fservice::${PROTOS_LIB_NAME} ALIAS ${PROTOS_LIB_NAME})
target_compile_features(${PROTOS_LIB_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROTOS_LIB_NAME}
  PUBLIC
  ${GRPC_GRPCPP}
  ${PROTOBUF_LIBPROTOBUF}
)

# Includes for Protobuf autogen
target_include_directories(${PROTOS_LIB_NAME}
  BEFORE
  PUBLIC
  # Include generated *.pb.h files
  "${CMAKE_CURRENT_BINARY_DIR}"
)

# Project lib configuration
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
    "fservice/AsyncServer.h"
    "fservice/AsyncServer.cpp"
    "fservice/IServerEventHandler.h"
    "fservice/IEngineEventHandler.h"
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
target_link_libraries(${LIB_NAME}
  PUBLIC
  fservice::${PROTOS_LIB_NAME}
  fmt::fmt
  log4cplus::log4cplusS
  Folly::folly
)

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
    find_package(Catch2 CONFIG REQUIRED)
    find_package(trompeloeil CONFIG REQUIRED)

    set(TEST_LIB_NAME "${LIB_NAME}Test")

    set(TEST_SRC_LIST
        "fservice/tests/EnumUtilTest.cpp"
        "fservice/tests/PathUtilTest.cpp"
        "fservice/tests/ScopeGuardTest.cpp"
        "fservice/tests/SyncClient.h"
        "fservice/tests/SyncClient.cpp"
        "fservice/tests/AsyncClient.h"
        "fservice/tests/AsyncClient.cpp"
        "fservice/tests/AsyncServerTest.cpp"
        "fservice/tests/IServerEventHandlerMock.h"
    )

    add_library(${TEST_LIB_NAME} OBJECT ${TEST_SRC_LIST})
    add_library(fservice::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

    add_sanitizers(${TEST_LIB_NAME})
    list(APPEND LCOV_REMOVE_PATTERNS "'*/tests/*'")
    add_coverage(${TEST_LIB_NAME})

    target_include_directories(${TEST_LIB_NAME} PRIVATE tests)
    target_compile_features(${TEST_LIB_NAME} PRIVATE cxx_std_17)
    target_link_libraries(${TEST_LIB_NAME} PUBLIC fservice::${LIB_NAME} Catch2::Catch2 trompeloeil)

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