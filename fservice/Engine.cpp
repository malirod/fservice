// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>

#include <fservice/AsyncServer.h>
#include <fservice/IEngineEventHandler.h>
#include <fservice/RepeatableTimeout.h>
#include <protos/Greeter.grpc.pb.h>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <cassert>
#include <utility>

namespace fservice {

Engine::Engine(folly::SocketAddress address,
               folly::EventBase& mainEventBase,
               IEngineEventHandler& engineEventHandler)
    : address_(std::move(address)),
      mainEventBase_(mainEventBase),
      engineEventHandler_(engineEventHandler) {
  LOG_AUTO_TRACE();
  LOG_INFO("Engine has been created.");
}

Engine::~Engine() {
  LOG_AUTO_TRACE();

  stopped_ = true;

  LOG_INFO("Engine has been destroyed.");
}

void Engine::start() {
  LOG_AUTO_TRACE();
  LOG_INFO("Starting engine");
  assert(initiated_);

  using std::chrono::milliseconds;

  mainEventBase_.runInLoop([this]() { engineEventHandler_.onEngineStarted(); });

  auto& timer = mainEventBase_.timer();
  timeout_ = std::make_unique<RepeatableTimeout>(
      timer, [this]() { publishStats(); }, milliseconds(4000));

  stopped_ = false;

  server_ = std::make_unique<AsyncServer>(mainEventBase_, *this);

  server_->runAsync(
      fmt::format("{}:{}", address_.getAddressStr(), address_.getPort()));

  LOG_INFO("Engine has been launched.");
  return;
}

void Engine::stop() {
  LOG_AUTO_TRACE();
  LOG_INFO("Stopping engine");
  assert(initiated_);

  if (stopped_) {
    LOG_INFO("Already stopped. Skip.");
    return;
  }

  stopped_ = true;
  LOG_INFO("Stopping server");
  server_.reset();
  LOG_INFO("Stopped server");

  mainEventBase_.runInLoop([this]() { engineEventHandler_.onEngineStopped(); });

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

// void Engine::processEvents() {
// }

void Engine::onSayHello(HelloRequest const& request, HelloReply& reply) {
  LOG_AUTO_TRACE();
  LOG_INFOF("Got message: {}", request.name());
  auto const prefix = std::string{"Hello "};
  reply.set_message(prefix + request.name());
}

} // namespace fservice