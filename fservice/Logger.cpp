// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/Logger.h>
#ifndef DISABLE_LOGGER

#include <log4cplus/config.hxx>
#include <log4cplus/configurator.h>
#include <log4cplus/ndc.h>

#include <fstream>

namespace {

void InitLogging(std::istream& logConfig) {
  log4cplus::initialize();

  log4cplus::PropertyConfigurator configurator(logConfig);
  configurator.configure();
}

} // namespace

IMPL_LOGGER_NAMESPACE_::LogManager::LogManager(
    std::string const& configFilePath) {
  std::ifstream logConfig(configFilePath);
  if (!logConfig.is_open()) {
    return;
  }
  InitLogging(logConfig);
}

IMPL_LOGGER_NAMESPACE_::LogManager::LogManager(std::istream& logConfig) {
  InitLogging(logConfig);
}

IMPL_LOGGER_NAMESPACE_::LogManager::~LogManager() {
  shutdown();
}

void IMPL_LOGGER_NAMESPACE_::LogManager::shutdown() {
  log4cplus::Logger::shutdown();
}

IMPL_LOGGER_NAMESPACE_::NDCWrapper::NDCWrapper(std::string const& msg) {
  log4cplus::getNDC().push(msg);
}

IMPL_LOGGER_NAMESPACE_::NDCWrapper::~NDCWrapper() {
  log4cplus::getNDC().pop_void();
}

#endif
