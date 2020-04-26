// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <folly/Expected.h>
#include <folly/SocketAddress.h>

#include <stdint.h>
#include <string>

namespace fservice {

struct StartupConfig {
  folly::SocketAddress const address;

  std::uint32_t const threadsCount = 0u;
};

folly::Expected<StartupConfig, GeneralError> processCmdArgs(int argc,
                                                            char** argv);

} // namespace fservice