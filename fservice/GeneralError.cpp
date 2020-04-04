// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/EnumUtil.h>
#include <fservice/GeneralError.h>

namespace fservice {

template <>
EnumStrings<GeneralError>::DataType EnumStrings<GeneralError>::data = {
    "Success",
    "Internal error",
    "Wrong startup parameter(s)",
    "Startup has failed",
    "Operation interrupted"};

const boost::system::error_category& detail::ErrorCategory::get() {
  static ErrorCategory instance;
  return instance;
}
boost::system::error_code make_error_code(GeneralError error) noexcept {
  return {ToIntegral(error), detail::ErrorCategory::get()};
}

const char* detail::ErrorCategory::name() const noexcept {
  return "GeneralError";
}

std::string detail::ErrorCategory::message(int error_value) const {
  return EnumToString(FromIntegral<GeneralError>(error_value));
}

} // namespace fservice