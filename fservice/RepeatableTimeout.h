// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <folly/io/async/HHWheelTimer.h>

#include <chrono>

namespace fservice {

class RepeatableTimeout : public folly::HHWheelTimer::Callback {
 public:
  /**
   * @brief Timeout handler which is called when timeout occurs.
   *
   */
  using OnTimeoutHandler = std::function<void()>;

  RepeatableTimeout(folly::HHWheelTimer& timer,
                    OnTimeoutHandler handler,
                    std::chrono::milliseconds timeout)
      : timer_(timer), handler_(std::move(handler)), timeout_(timeout) {
    timer_.scheduleTimeout(this, timeout_);
  }

  void timeoutExpired() noexcept override {
    handler_();
    timer_.scheduleTimeout(this, timeout_);
  }

 private:
  folly::HHWheelTimer& timer_;

  const OnTimeoutHandler handler_;

  const std::chrono::milliseconds timeout_;
};

} // namespace fservice