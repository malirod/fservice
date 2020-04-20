// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

namespace fservice {

class HelloRequest;
class HelloReply;

struct IServerEventHandler {
  virtual ~IServerEventHandler() = default;

  virtual void onSayHello(HelloRequest const& request, HelloReply& reply) = 0;
};

} // namespace fservice