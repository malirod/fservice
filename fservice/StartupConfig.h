// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/Outcome.h>

#include <folly/SocketAddress.h>

#include <stdint.h>
#include <string>

namespace fservice {

struct StartupConfig {
  folly::SocketAddress const address;

  std::uint32_t const threadsCount = 0u;
};

outcome::result<StartupConfig> processCmdArgs(int argc, char** argv);

} // namespace fservice