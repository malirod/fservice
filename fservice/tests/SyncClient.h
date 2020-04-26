// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/GeneralError.h>
#include <fservice/Logger.h>
#include <protos/Greeter.grpc.pb.h>

#include <folly/Expected.h>

#include <atomic>

#include <grpcpp/grpcpp.h>

namespace fservice {

/**
 * Implementation of sync Grpc client.
 */
class SyncClient final {
 public:
  explicit SyncClient(std::shared_ptr<grpc::Channel> channel);

  /* Assembles the client's payload, sends it and presents the response back
   * from the server. */
  folly::Expected<std::string, GeneralError> SayHello(std::string const& user);

 private:
  DECLARE_GET_LOGGER("SyncClient")

  /* Out of the passed in Channel comes the stub, stored here, our view of the
   * server's exposed services. */
  std::unique_ptr<Greeter::Stub> stub_;
};

} // namespace fservice