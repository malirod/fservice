// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Logger.h>
#include <fservice/PathUtil.h>

#include <trompeloeil.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
  INIT_LOGGER(fservice::getExePath(argv[0]).replace_filename("logger.cfg"));

  // Configure Mocking to work with Catch2
  trompeloeil::set_reporter([](trompeloeil::severity s,
                               const char* file,
                               unsigned long line,
                               std::string const& msg) {
    std::ostringstream os;
    if (line)
      os << file << ':' << line << '\n';
    os << msg;
    auto failure = os.str();
    if (s == trompeloeil::severity::fatal) {
      FAIL(failure);
    } else {
      CAPTURE(failure);
      CHECK(failure.empty());
    }
  });

  auto const result = Catch::Session().run(argc, argv);
  return result;
}