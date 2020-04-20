// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/Logger.h>

#include <protos/Greeter.grpc.pb.h>

#include <grpcpp/grpcpp.h>

#include <thread>

namespace folly {

class EventBase;

} // namespace folly

namespace fservice {

class IServerEventHandler;

/* Grps Async server */
class AsyncServer final {
 public:
  AsyncServer(folly::EventBase& eventLoop,
              IServerEventHandler& serverEventHandler);

  ~AsyncServer();

  void runAsync(std::string const& address);

 private:
  /* Sync call to stop server. */
  void stop();

  /* Holds context of client request. */
  class CallData {
   public:
    CallData(folly::EventBase* eventLoop,
             Greeter::AsyncService* service,
             grpc::ServerCompletionQueue* completionQueue,
             IServerEventHandler* serverEventHandler);

    void proceed(bool const ok);

   private:
    DECLARE_GET_LOGGER("Server.CallData")

    folly::EventBase* eventLoop_;

    Greeter::AsyncService* service_;

    /* The producer-consumer queue where for asynchronous server
     * notifications.*/
    grpc::ServerCompletionQueue* completionQueue_;

    IServerEventHandler* serverEventHandler_;

    /* Context for the rpc, allowing to tweak aspects of it such as the use of
     * compression, authentication, as well as to send metadata back to the
     * client. */
    grpc::ServerContext context_;

    /* Request from the client. */
    HelloRequest request_;

    /* Response to the client. */
    HelloReply reply_;

    /* The means to get back to the client. */
    grpc::ServerAsyncResponseWriter<HelloReply> responder_;

    /* Request states */
    enum class CallStatus { CREATE, PROCESS, FINISH };

    /*The current serving state. */
    CallStatus status_;
  };

  DECLARE_GET_LOGGER("Server")

  /* Check pending Rpcs. This can be run in multiple threads if needed. */
  void handleRpcs();

  folly::EventBase& eventLoop_;

  IServerEventHandler& serverEventHandler_;

  std::unique_ptr<grpc::ServerCompletionQueue> completionQueue_;

  Greeter::AsyncService greeterAsyncService_;

  std::unique_ptr<grpc::Server> grpcServer_;

  std::thread workerThread_;
};

} // namespace fservice