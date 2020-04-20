// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/AsyncServer.h>

#include <fservice/IServerEventHandler.h>

#include <folly/io/async/EventBase.h>

namespace fservice {

AsyncServer::AsyncServer(folly::EventBase& eventLoop,
                         IServerEventHandler& serverEventHandler)
    : eventLoop_(eventLoop), serverEventHandler_(serverEventHandler) {
}

AsyncServer::~AsyncServer() {
  LOG_AUTO_TRACE();
  stop();
}

void AsyncServer::runAsync(std::string const& address) {
  LOG_AUTO_TRACE();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&greeterAsyncService_);
  completionQueue_ = builder.AddCompletionQueue();
  grpcServer_ = builder.BuildAndStart();
  LOG_INFOF("Server listening on {}", address);
  // Proceed to the server's main loop.
  // Spawn reader thread that loops indefinitely
  workerThread_ = std::thread(&AsyncServer::handleRpcs, this);
}

void AsyncServer::stop() {
  LOG_AUTO_TRACE();
  assert(workerThread_.joinable());

  grpcServer_->Shutdown();
  // Always shutdown the completion queue after the server.
  completionQueue_->Shutdown();

  workerThread_.join();
}

AsyncServer::CallData::CallData(folly::EventBase* eventLoop,
                                Greeter::AsyncService* service,
                                grpc::ServerCompletionQueue* completionQueue,
                                IServerEventHandler* serverEventHandler)
    : eventLoop_(eventLoop),
      service_(service),
      completionQueue_(completionQueue),
      serverEventHandler_(serverEventHandler),
      responder_(&context_),
      status_(CallStatus::CREATE) {
  proceed(true);
}

void AsyncServer::CallData::proceed(bool const ok) {
  if (ok && status_ == CallStatus::CREATE) {
    // Make this instance progress to the PROCESS state.
    status_ = CallStatus::PROCESS;

    // As part of the initial CREATE state, we *request* that the system
    // start processing SayHello requests. In this request, "this" acts are
    // the tag uniquely identifying the request (so that different CallData
    // instances can serve different requests concurrently), in this case
    // the memory address of this CallData instance.
    service_->RequestSayHello(&context_,
                              &request_,
                              &responder_,
                              completionQueue_,
                              completionQueue_,
                              this);
  } else if (ok && status_ == CallStatus::PROCESS) {
    LOG_TRACE("Processing request");
    // Spawn a new CallData instance to serve new clients while we process
    // the one for this CallData. The instance will deallocate itself as
    // part of its FINISH state.
    new CallData(eventLoop_, service_, completionQueue_, serverEventHandler_);

    // Handle request in the event loop
    eventLoop_->runInEventBaseThread([this]() {
      serverEventHandler_->onSayHello(request_, reply_);

      // And we are done! Let the gRPC runtime know we've finished, using
      // the memory address of this instance as the uniquely identifying tag
      // for the event.
      status_ = CallStatus::FINISH;
      responder_.Finish(reply_, grpc::Status::OK, this);
    });
  } else {
    // Not ok or CallStatus::FINISH
    // Once in the FINISH state, deallocate ourselves (CallData).
    delete this;
  }
}

void AsyncServer::handleRpcs() {
  // Spawn a new CallData instance to serve new clients.
  new CallData(&eventLoop_,
               &greeterAsyncService_,
               completionQueue_.get(),
               &serverEventHandler_);
  void* tag; // uniquely identifies a request.
  bool ok;

  // Block waiting to read the next event from the completion queue. The
  // event is uniquely identified by its tag, which in this case is the
  // memory address of a CallData instance.
  // The return value of Next should always be checked. This return value
  // tells us whether there is any kind of event or completionQueue_ is
  // shutting down.
  while (completionQueue_->Next(&tag, &ok)) {
    auto* callData = static_cast<CallData*>(tag);
    callData->proceed(ok);
  }
}

} // namespace fservice