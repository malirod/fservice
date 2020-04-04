// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>

#include <fservice/RepeatableTimeout.h>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <cassert>
#include <utility>

namespace fservice {

Engine::Engine(folly::SocketAddress address, folly::EventBase* mainEventBase)
    : address_(std::move(address)), mainEventBase_(mainEventBase) {
  LOG_AUTO_TRACE();
  assert(mainEventBase_ != nullptr && "eventBase is not set");
  LOG_INFO("Engine has been created.");
}

Engine::~Engine() {
  LOG_AUTO_TRACE();

  stopped_ = true;

  LOG_INFO("Engine has been destroyed.");
}

void Engine::start(OnStartedHandler onStarted) {
  LOG_AUTO_TRACE();
  LOG_INFO("Starting engine");
  assert(initiated_);

  using std::chrono::milliseconds;

  mainEventBase_->runInLoop(
      [onStarted = std::move(onStarted)]() { onStarted(); });

  auto& timer = mainEventBase_->timer();
  timeout_ = std::make_unique<RepeatableTimeout>(
      timer, [this]() { publishStats(); }, milliseconds(4000));

  LOG_INFO("Engine has been launched.");
  return;
}

void Engine::stop(OnStoppedHandler onStopped) {
  LOG_AUTO_TRACE();
  LOG_INFO("Stopping engine");
  assert(initiated_);

  if (stopped_) {
    LOG_INFO("Already stopped. Skip.");
    return;
  }

  stopped_ = true;

  mainEventBase_->runInLoop(
      [onStopped = std::move(onStopped)]() { onStopped(); });

  return;
}

bool Engine::init() {
  LOG_AUTO_TRACE();
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

void Engine::publishStats() {
  LOG_AUTO_TRACE();
  assert(initiated_);
  LOG_INFO("Publishing periodical stats");
}

} // namespace fservice