// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>
#include <fservice/EngineLauncher.h>
#include <fservice/GeneralError.h>
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
  engine_->stop([this]() { this->onEngineStopped(); });
}

void EngineLauncher::onEngineStarted() {
  LOG_INFO("Engine started");
  assert(mainEventBase_ != nullptr);
}

void EngineLauncher::onEngineStopped() {
  LOG_INFO("Engine stopped");
  mainEventBase_->terminateLoopSoon();
}

ErrorCode EngineLauncher::init() {
  LOG_AUTO_TRACE();

  LOG_INFOF("Address: {}:{}; Threads: {}",
            startupConfig_.address.getAddressStr(),
            startupConfig_.address.getPort(),
            startupConfig_.threadsCount);

  signalHandler_ =
      std::make_unique<SignalHandler>([this]() { onTerminationRequest(); });
  signalHandler_->install({SIGINT, SIGTERM});

  auto cpuThreadExecutor = std::make_shared<folly::CPUThreadPoolExecutor>(
      startupConfig_.threadsCount,
      std::make_shared<folly::NamedThreadFactory>("CPUThread"));
  folly::setCPUExecutor(cpuThreadExecutor);

  mainEventBase_ = folly::EventBaseManager::get()->getEventBase();

  engine_ = std::make_unique<Engine>(startupConfig_.address, mainEventBase_);

  auto const initiated = engine_->init();
  return initiated ? make_error_code(GeneralError::Success)
                   : make_error_code(GeneralError::StartupFailed);
}

void EngineLauncher::deInit() {
  LOG_AUTO_TRACE();
  mainEventBase_ = nullptr;
}

ErrorCode EngineLauncher::doRun() {
  LOG_AUTO_TRACE();
  assert(mainEventBase_ != nullptr);

  engine_->start([this]() { onEngineStarted(); });

  LOG_INFO("Waiting for termination request");
  mainEventBase_->loopForever();

  return GeneralError::Success;
}

ErrorCode EngineLauncher::run() {
  LOG_AUTO_TRACE();

  const auto errorCode = init();
  fservice::ScopeGuard const guard{[&]() { deInit(); }};
  return errorCode ? errorCode : doRun();
}

} // namespace fservice