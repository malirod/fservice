// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/ScopeGuard.h>

#include <catch2/catch.hpp>

TEST_CASE("Run lambda on scope exit", "[ScopeGuard]") {
  int hitCount = 0;
  {
    auto action = [&]() { ++hitCount; };
    fservice::ScopeGuard const guard{action};
    REQUIRE(hitCount == 0);
  }
  REQUIRE(hitCount == 1);
}