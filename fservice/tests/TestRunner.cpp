// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Logger.h>
#include <fservice/PathUtil.h>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <nngpp/nngpp.h>

int main(int argc, char* argv[]) {
  INIT_LOGGER(fservice::getExePath(argv[0]).replace_filename("logger.cfg"));
  auto const result = Catch::Session().run(argc, argv);
  nng::fini(); // cleanup lib to prevent complains from memory checkers
  return result;
}