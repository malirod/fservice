// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/tests/AsyncClient.h>

namespace fservice {

AsyncClient::AsyncClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Greeter::NewStub(channel)) {
}

void AsyncClient::SayHello(std::string const& user) {
  LOG_INFOF("!!! Sending: {}", user);
  // Data we are sending to the server.
  HelloRequest request;
  request.set_name(user);

  // Call object to store rpc data
  auto* call = new AsyncClientCall;

  // stub_->PrepareAsyncSayHello() creates an RPC object, returning
  // an instance to store in "call" but does not actually start the RPC
  // Because we are using the asynchronous API, we need to hold on to
  // the "call" instance in order to get updates on the ongoing RPC.
  call->responseReader =
      stub_->PrepareAsyncSayHello(&call->context, request, &completionQueue_);

  // StartCall initiates the RPC call
  call->responseReader->StartCall();

  // Request that, upon completion of the RPC, "reply" be updated with the
  // server's response; "status" with the indication of whether the operation
  // was successful. Tag the request with the memory address of the call object.
  call->responseReader->Finish(&call->reply, &call->status, (void*)call);
}

void AsyncClient::AsyncCompleteRpc() {
  void* gotTag;
  bool ok = false;

  // Block until the next result is available in the completion queue.
  while (completionQueue_.Next(&gotTag, &ok)) {
    // The tag  is the memory location of the call object
    AsyncClientCall* call = static_cast<AsyncClientCall*>(gotTag);

    // Verify that the request was completed successfully. Note that "ok"
    // corresponds solely to the request for updates introduced by Finish().
    GPR_ASSERT(ok);

    if (call->status.ok()) {
      LOG_INFOF("Greeter received: {}", call->reply.message());
    } else {
      LOG_INFO("RPC failed");
    }

    // Once we're complete, deallocate the call object.
    delete call;
  }
}

} // namespace fservice