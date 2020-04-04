// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/PathUtil.h>

#include <catch2/catch.hpp>

TEST_CASE("Full path", "[PathUtil]") {
  char argv0[] = "/home/user/myapp";
  char *argv[] = {argv0, NULL};

  auto const filePath = fservice::getExePath(argv[0]);
  REQUIRE(filePath.has_filename());
  REQUIRE(filePath.stem() == "myapp");
}

TEST_CASE("Full path with params", "[PathUtil]") {
  char argv0[] = "/home/user/myapp";
  char argv1[] = "arg1";
  char argv2[] = "arg2";

  char *argv[] = {argv0, argv1, argv2, NULL};

  auto const filePath = fservice::getExePath(argv[0]);
  REQUIRE(filePath.has_filename());
  REQUIRE(filePath.stem() == "myapp");
}

TEST_CASE("Rel path", "[PathUtil]") {
  char argv0[] = "../user/myapp";
  char argv1[] = "arg1";
  char argv2[] = "arg2";

  char *argv[] = {argv0, argv1, argv2, NULL};

  auto const filePath = fservice::getExePath(argv[0]);
  REQUIRE(filePath.has_filename());
  REQUIRE(filePath.stem() == "myapp");
}

TEST_CASE("Empty path", "[PathUtil]") {
  char argv0[] = "";

  char *argv[] = {argv0, NULL};

  auto const filePath = fservice::getExePath(argv[0]);
  REQUIRE(!filePath.has_filename());
  REQUIRE(filePath.stem().empty());
}

TEST_CASE("Append to empty path", "[PathUtil]") {
  char argv0[] = "";

  char *argv[] = {argv0, NULL};

  auto const filePath = fservice::getExePath(argv[0]);
  REQUIRE(!filePath.has_filename());
  REQUIRE(filePath.stem().empty());
}