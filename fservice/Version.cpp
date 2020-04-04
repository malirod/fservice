// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Version.h>

#include <fmt/format.h>

namespace fservice {

std::string getVersion() {
  return fmt::format("{}.{}.{}", kMajor, kMinor, kPatch);
}

} // namespace fservice