// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/Logger.h>

#include <folly/SocketAddress.h>

#include <nngpp/nngpp.h>

#include <atomic>

namespace folly {

class EventBase;

} // namespace folly

namespace nng {

class socket;

} // namespace nng

namespace fservice {

class RepeatableTimeout;
struct Work;

/**
 * Implementation of Engine. Holds all and runs all business logic.
 */
class Engine {
 public:
  /**
   * Creates instance of Engine.
   * @param address Engine address.
   */
  explicit Engine(folly::SocketAddress address,
                  folly::EventBase* mainEventBase);

  Engine& operator=(Engine const&) = delete;
  Engine(Engine const&) = delete;

  /**
   * Destroy Engine instance. Mark Engine as stopped. Don't perform actual
   * shutdown.
   */
  ~Engine();

  using OnStartedHandler = std::function<void()>;
  using OnStoppedHandler = std::function<void()>;

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed
   * asynchronously.
   */
  void start(OnStartedHandler onStarted);

  /**
   * Trigger stop sequence. Non-blocking.
   */
  void stop(OnStoppedHandler onStopped);

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  bool init();

  struct Work {
    using OnWorkHandler = std::function<void(Work& work)>;

    enum class State { INIT, RECV, WAIT, SEND } state = State::INIT;
    nng::aio aio;
    nng::msg msg;
    nng::ctx ctx;
    const OnWorkHandler onWork;

    Work(nng::socket_view sock, OnWorkHandler onWorkHandler);
  };

 private:
  DECLARE_GET_LOGGER("Engine")

  void publishStats();

  void onReqWork(Work& work);

  bool initiated_ = false;

  folly::SocketAddress const address_;

  std::atomic_bool stopped_ = false;

  folly::EventBase* mainEventBase_ = nullptr;

  std::unique_ptr<RepeatableTimeout> timeout_;

  std::unique_ptr<nng::socket> replySocket_;

  std::vector<std::unique_ptr<Work>> replyWorkers_;
};

} // namespace fservice
