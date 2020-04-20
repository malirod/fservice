// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <fservice/IServerEventHandler.h>

#include <trompeloeil.hpp>

namespace fservice {

class ServerEventHandlerMock : public IServerEventHandler {
 public:
  MAKE_MOCK2(onSayHello,
             void(HelloRequest const& request, HelloReply& reply),
             override);
};

} // namespace fservice