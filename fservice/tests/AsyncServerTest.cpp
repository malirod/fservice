// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/AsyncServer.h>
#include <fservice/GeneralError.h>
#include <fservice/Logger.h>
#include <fservice/tests/IServerEventHandlerMock.h>
#include <fservice/tests/SyncClient.h>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>

#include <catch2/catch.hpp>

DECLARE_GLOBAL_GET_LOGGER("ServerTest")

TEST_CASE("Sync request and Async response", "[AsyncServer]") {
  using trompeloeil::_;

  fservice::ServerEventHandlerMock fakeServerEventHandler;
  ALLOW_CALL(fakeServerEventHandler, onSayHello(_, _)).SIDE_EFFECT({
    LOG_INFOF("Server got request: {}", _1.name());
    _2.set_message("Hello " + _1.name());
  });

  auto* eventLoop = folly::EventBaseManager::get()->getEventBase();
  auto const address = std::string{"127.0.0.1:12001"};
  auto server = fservice::AsyncServer(*eventLoop, fakeServerEventHandler);
  server.runAsync(address);

  auto clientThread = std::thread([address = std::move(address), eventLoop]() {
    auto client = fservice::SyncClient(
        grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    for (int i = 1; i <= 5; ++i) {
      auto const user = std::string{"world " + std::to_string(i)};
      auto const replyOrError = client.SayHello(user);
      REQUIRE(replyOrError.has_value());
      REQUIRE(replyOrError.value() == "Hello " + user);
    }
    eventLoop->terminateLoopSoon();
  });

  eventLoop->loopForever();
  clientThread.join();
}

TEST_CASE("Client connect when no server available", "[AsyncServer]") {
  auto const address = std::string{"127.0.0.1:12001"};

  auto client = fservice::SyncClient(
      grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

  auto const user = std::string{"world "};
  auto const replyOrError = client.SayHello(user);
  REQUIRE(replyOrError.has_error());
  REQUIRE(replyOrError.error() == fservice::GeneralError::RpcFailed);
}