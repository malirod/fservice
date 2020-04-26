// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/Engine.h>
#include <fservice/GeneralError.h>
#include <fservice/IEngineEventHandler.h>
#include <fservice/Logger.h>
#include <fservice/SignalHandler.h>
#include <fservice/StartupConfig.h>

#include <memory>

namespace folly {

class EventBase;

} // namespace folly

namespace fservice {

/**
 * Encapsulates logic of initialization of Engine and startup.
 */
class EngineLauncher final : public IEngineEventHandler {
 public:
  /**
   * Create ready to use instance of EngineLauncher.
   * @param startupConfig Configuration.
   */
  explicit EngineLauncher(StartupConfig startupConfig);

  EngineLauncher(EngineLauncher const&) = delete;
  EngineLauncher(EngineLauncher const&&) = delete;
  EngineLauncher& operator=(EngineLauncher const&) = delete;
  EngineLauncher& operator=(EngineLauncher const&&) = delete;

  /**
   * Setup environment, run Engine and wait until it will finish.
   * @return Execution result.
   */
  std::error_code run();

 private:
  DECLARE_GET_LOGGER("EngineLauncher")

  std::error_code init();

  void deInit();

  std::error_code doRun();

  void onTerminationRequest();

  void onEngineStarted() override;

  void onEngineStopped() override;

  StartupConfig const startupConfig_;

  /**
   * Signal handler which will shutdown Engine
   */
  std::unique_ptr<SignalHandler> signalHandler_;

  std::unique_ptr<Engine> engine_;

  folly::EventBase* mainEventBase_ = nullptr;

  bool stopped_ = false;

  // std::unique_ptr<ThreadPool> thread_pool_main_;

  // std::unique_ptr<ThreadPool> thread_pool_net_;
};

} // namespace fservice
