// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/PathUtil.h>

namespace fservice {

std::filesystem::path getExePath(char const* const argv0) {
  auto exePath =
      std::filesystem::weakly_canonical(std::filesystem::path(argv0));
  return exePath;
}

} // namespace fservice