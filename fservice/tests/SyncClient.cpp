// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/GeneralError.h>
#include <fservice/tests/SyncClient.h>

namespace fservice {

SyncClient::SyncClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Greeter::NewStub(channel)) {
}

outcome::result<std::string> SyncClient::SayHello(std::string const& user) {
  LOG_TRACEF("Sending: {}", user);
  // Data we are sending to the server.
  HelloRequest request;
  request.set_name(user);

  // Container for the data we expect from the server.
  HelloReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  grpc::ClientContext context;

  // The actual RPC
  auto status = stub_->SayHello(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    return reply.message();
  } else {
    LOG_ERRORF("Error: {}:{}", status.error_code(), status.error_message());
    return GeneralError::RpcFailed;
  }
}

} // namespace fservice