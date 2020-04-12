// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>

#include <fservice/RepeatableTimeout.h>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <fmt/format.h>

#include <nngpp/protocol/rep0.h>

#include <cassert>
#include <functional>
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
      timer, [this]() { publishStats(); }, milliseconds(10000));

  // Setup reply socket
  replySocket_ = std::make_unique<nng::socket>(nng::rep::open());
  const int replyWorkersCount = 1;
  replyWorkers_.reserve(replyWorkersCount);
  for (int i = 0; i < replyWorkersCount; ++i) {
    replyWorkers_.push_back(std::make_unique<Work>(
        *replySocket_, [this](Work& work) { onReqWork(work); }));
  }
  auto const url =
      fmt::format("tcp://{}:{}", address_.getAddressStr(), address_.getPort());
  LOG_INFOF("Repl listening on {}", url);
  replySocket_->listen(url.c_str());

  for (auto& work : replyWorkers_) {
    onReqWork(*work);
  }

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

  replyWorkers_.clear();

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

void onWorkHandlerHelper(void* arg);

Engine::Work::Work(nng::socket_view sock, OnWorkHandler onWorkHandler)
    : aio(onWorkHandlerHelper, this), ctx(sock), onWork(onWorkHandler) {
}

void onWorkHandlerHelper(void* arg) {
  assert(arg);
  auto* work = static_cast<Engine::Work*>(arg);
  work->onWork(*work);
}

void Engine::onReqWork(Work& work) {
  LOG_AUTO_TRACE();
  switch (work.state) {
    case Work::State::INIT: {
      LOG_DEBUG("State INIT");
      work.state = Work::State::RECV;
      work.ctx.recv(work.aio);
      break;
    }
    case Work::State::RECV: {
      LOG_DEBUG("State RECV");
      auto result = work.aio.result();
      LOG_DEBUGF("RECV result: {}", nng::to_string(result));
      if (result == nng::error::closed) {
        return;
      }
      if (result != nng::error::success) {
        throw nng::exception(result);
      }
      auto const reqMsg = work.aio.release_msg();
      std::string const header{static_cast<char*>(reqMsg.header().data()),
                               reqMsg.header().size()};
      std::string const body{static_cast<char*>(reqMsg.body().data()),
                             reqMsg.body().size()};
      LOG_DEBUGF("Received message: header={}, body={}", header, body);
      try {
        auto const repMsg = nng::make_msg(0u);
        repMsg.header().append("rep");
        repMsg.body().append("Hello!");
        work.msg = std::move(repMsg);
      } catch (const nng::exception&) {
        // bad message, just ignore it.
        work.ctx.recv(work.aio);
        return;
      }
      work.state = Work::State::WAIT;
      nng::sleep(5, work.aio); // emulate workload
      break;
    }
    case Work::State::WAIT: {
      LOG_DEBUG("State WAIT");
      std::string const header{static_cast<char*>(work.msg.header().data()),
                               work.msg.header().size()};
      std::string const body{static_cast<char*>(work.msg.body().data()),
                             work.msg.body().size()};
      LOG_INFOF("Sending message to client: header={}, body={}", header, body);
      work.aio.set_msg(std::move(work.msg));
      work.state = Work::State::SEND;
      work.ctx.send(work.aio);
      break;
    }
    case Work::State::SEND: {
      LOG_DEBUG("State SEND");
      auto result = work.aio.result();
      LOG_DEBUGF("SEND result: {}", nng::to_string(result));
      if (result != nng::error::success) {
        throw nng::exception(result);
      }
      work.state = Work::State::RECV;
      work.ctx.recv(work.aio);
      break;
    }
    default:
      throw nng::exception(nng::error::state);
  }
}

} // namespace fservice