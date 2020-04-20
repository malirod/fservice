// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/IServerEventHandler.h>
#include <fservice/Logger.h>

#include <folly/SocketAddress.h>

#include <atomic>

namespace folly {

class EventBase;

} // namespace folly

namespace fservice {

class RepeatableTimeout;

class AsyncServer;

struct IEngineEventHandler;

/**
 * Implementation of Engine. Holds all and runs all business logic.
 */
class Engine final : public IServerEventHandler {
 public:
  /**
   * Creates instance of Engine.
   * @param address Engine address.
   */
  explicit Engine(folly::SocketAddress address,
                  folly::EventBase& mainEventBase,
                  IEngineEventHandler& engineEventHandler);

  Engine& operator=(Engine const&) = delete;
  Engine(Engine const&) = delete;

  /**
   * Destroy Engine instance. Mark Engine as stopped. Don't perform actual
   * shutdown.
   */
  ~Engine();

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed
   * asynchronously.
   */
  void start();

  /**
   * Trigger stop sequence. Non-blocking.
   */
  void stop();

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  bool init();

  // void processEvents();

  void onSayHello(HelloRequest const& request, HelloReply& reply) override;

 private:
  DECLARE_GET_LOGGER("Engine")

  void publishStats();

  bool initiated_ = false;

  folly::SocketAddress const address_;

  std::atomic_bool stopped_ = false;

  folly::EventBase& mainEventBase_;

  IEngineEventHandler& engineEventHandler_;

  std::unique_ptr<RepeatableTimeout> timeout_;

  std::unique_ptr<AsyncServer> server_;
};

} // namespace fservice
