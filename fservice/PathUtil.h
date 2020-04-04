// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <filesystem>

namespace fservice {

std::filesystem::path getExePath(char const* const argv0);

} // namespace fservice