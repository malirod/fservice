// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Engine.h>
#include <fservice/Logger.h>

#include <folly/SocketAddress.h>
#include <folly/io/async/EventBaseManager.h>

#include <catch2/catch.hpp>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>

#include <fmt/format.h>

#include <boost/thread/barrier.hpp>

DECLARE_GLOBAL_GET_LOGGER("EngineTest")

TEST_CASE("Start engine and make request", "[Engine]") {
  LOG_DEBUG("Starting test");
  boost::barrier barrier{2};

  folly::SocketAddress const address{"127.0.0.1", 12000};

  auto* mainEventBase = folly::EventBaseManager::get()->getEventBase();

  auto engine = std::make_unique<fservice::Engine>(address, mainEventBase);
  auto initiated = engine->init();
  REQUIRE(initiated);
  engine->start([&barrier]() {
    LOG_DEBUG("Engine started");
    barrier.wait();
  });

  std::thread clientThread([address = std::move(address), &barrier]() {
    LOG_DEBUG("Waiting engine to be started");
    barrier.wait();
    auto sock = nng::req::open();
    REQUIRE(sock);
    auto const url =
        fmt::format("tcp://{}:{}", address.getAddressStr(), address.getPort());
    LOG_DEBUGF("Client dialing to url {}", url);
    sock.dial(url.c_str());

    auto reqMsg = nng::make_msg(0u);
    reqMsg.header().append("testreqmsg");
    reqMsg.body().append(R"({"item": "text data"})");
    std::string header{static_cast<char*>(reqMsg.header().data()),
                       reqMsg.header().size()};
    std::string body{static_cast<char*>(reqMsg.body().data()),
                     reqMsg.body().size()};
    LOG_DEBUGF("Sending message: header={}, body={}", header, body);
    sock.send(std::move(reqMsg));

    LOG_DEBUG("Receiving message");
    auto const repMsg = sock.recv_msg();
    header.assign(static_cast<char*>(repMsg.header().data()),
                  repMsg.header().size());
    body.assign(static_cast<char*>(repMsg.body().data()), repMsg.body().size());
    LOG_DEBUGF("Client received message: header={}, body={}", header, body);
    REQUIRE(repMsg.header().get() == "rep");
    REQUIRE(repMsg.body().get() == "Hello!");

    folly::EventBaseManager::get()->getEventBase()->terminateLoopSoon();
  });
  engine->stop([]() {});
  LOG_DEBUG("Waiting main loop");
  mainEventBase->loopForever();
  LOG_DEBUG("Waited main loop");

  clientThread.join();
  LOG_DEBUG("End of test");
}