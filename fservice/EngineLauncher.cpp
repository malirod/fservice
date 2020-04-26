// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>
#include <fservice/EngineLauncher.h>
#include <fservice/ScopeGuard.h>
#include <fservice/SignalHandler.h>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/EventBaseManager.h>

#include <csignal>

namespace fservice {

EngineLauncher::EngineLauncher(StartupConfig startupConfig)
    : startupConfig_(std::move(startupConfig)) {
}

void EngineLauncher::onTerminationRequest() {
  LOG_INFO("Termination request received. Stopping.");
  stopped_ = true;
  mainEventBase_->terminateLoopSoon();
  engine_->stop();
}

void EngineLauncher::onEngineStarted() {
  LOG_INFO("Engine started");
  assert(mainEventBase_ != nullptr);
}

void EngineLauncher::onEngineStopped() {
  LOG_INFO("Engine stopped");
}

std::error_code EngineLauncher::init() {
  LOG_AUTO_TRACE();

  LOG_INFOF("Address: {}:{}; Threads: {}",
            startupConfig_.address.getAddressStr(),
            startupConfig_.address.getPort(),
            startupConfig_.threadsCount);

  signalHandler_ =
      std::make_unique<SignalHandler>([this]() { onTerminationRequest(); });
  signalHandler_->install({SIGINT, SIGTERM});

  // Setup CPU executor
  // auto cpuThreadExecutor = std::make_shared<folly::CPUThreadPoolExecutor>(
  //     startupConfig_.threadsCount,
  //     std::make_shared<folly::NamedThreadFactory>("CPUThread"));
  // folly::setCPUExecutor(cpuThreadExecutor);

  mainEventBase_ = folly::EventBaseManager::get()->getEventBase();

  engine_ =
      std::make_unique<Engine>(startupConfig_.address, *mainEventBase_, *this);

  auto const initiated = engine_->init();
  return initiated ? GeneralError::Success : GeneralError::StartupFailed;
}

void EngineLauncher::deInit() {
  LOG_AUTO_TRACE();
  mainEventBase_ = nullptr;
}

std::error_code EngineLauncher::doRun() {
  LOG_AUTO_TRACE();
  assert(mainEventBase_ != nullptr);

  engine_->start();

  LOG_INFO("Waiting for termination request");
  mainEventBase_->loopForever();
  // Manual event loop (alternative to mainEventBase_->loopForever())
  // while (!stopped_) {
  // mainEventBase_->loopOnce(EVLOOP_NONBLOCK);
  // engine_->processEvents();
  //}

  return GeneralError::Success;
}

std::error_code EngineLauncher::run() {
  LOG_AUTO_TRACE();

  const auto errorCode = init();
  fservice::ScopeGuard const guard{[&]() { deInit(); }};
  return errorCode ? errorCode : doRun();
}

} // namespace fservice