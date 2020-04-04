// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/system/error_code.hpp>
#include <string>

namespace fservice {

/**
 * Enum which describes general Engine errors.
 */
enum class GeneralError {
  Success,
  InternalError,
  WrongStartupParams,
  StartupFailed,
  Interrupted
};

namespace detail {

/**
 * Error category for general errors.
 */
class ErrorCategory : public boost::system::error_category {
 public:
  /**
   * Gets category name.
   * @return Name of the category.
   */
  const char* name() const noexcept override;

  /**
   * Convert error code to corresponding message string.
   * @param error_value Error code
   * @return Error massage
   */
  std::string message(int error_value) const override;

  /**
   * Allows to get access to single instance of this category.
   * @return This Category.
   */
  static const boost::system::error_category& get();

 protected:
  ErrorCategory() = default;
};

} // namespace detail

// Overload the global make_error_code() free function with our
// custom enum. It will be found via ADL by the compiler if needed.
boost::system::error_code make_error_code(GeneralError error) noexcept;

} // namespace fservice

namespace boost { namespace system {
// Tell the C++ 11 STL metaprogramming that enum ConversionErrc
// is registered with the standard error code system
template <>
struct is_error_code_enum<fservice::GeneralError> : std::true_type {};
}} // namespace boost::system