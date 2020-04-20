// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/Logger.h>
#include <protos/Greeter.grpc.pb.h>

#include <atomic>

#include <grpcpp/grpcpp.h>

namespace fservice {

/**
 * Implementation of Grpc client.
 */
class AsyncClient final {
 public:
  explicit AsyncClient(std::shared_ptr<grpc::Channel> channel);

  /* Assembles the client's payload and sends it to the server. */
  void SayHello(std::string const& user);

  /* Loop while listening for completed responses. Prints out the response from
   * the server.
   */
  void AsyncCompleteRpc();

 private:
  DECLARE_GET_LOGGER("AsyncClient")

  /* struct for keeping state and data information. */
  struct AsyncClientCall {
    /* Container for the data we expect from the server. */
    HelloReply reply;

    /* Context for the client. It could be used to convey extra information to
     * the server and/or tweak certain RPC behaviors. */
    grpc::ClientContext context;

    /* Storage for the status of the RPC upon completion. */
    grpc::Status status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<HelloReply>> responseReader;
  };

  /* Out of the passed in Channel comes the stub, stored here, our view of the
   * server's exposed services. */
  std::unique_ptr<Greeter::Stub> stub_;

  /* The producer-consumer queue we use to communicate asynchronously with the
   * gRPC runtime. */
  grpc::CompletionQueue completionQueue_;
};

} // namespace fservice