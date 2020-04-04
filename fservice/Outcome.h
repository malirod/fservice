// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/outcome.hpp>
#include <boost/system/error_code.hpp>

namespace outcome = boost::outcome_v2;

namespace fservice {

using ErrorCode = boost::system::error_code;

} // namespace fservice