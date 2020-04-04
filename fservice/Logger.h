// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#define DOWHILE_NOTHING() \
  do {                    \
  } while (0)

#if defined(DISABLE_LOGGER)

#define INIT_LOGGER(log_config) DOWHILE_NOTHING()
#define SHUTDOWN_LOGGER() DOWHILE_NOTHING()

#define LOG_TRACEL(logger, message) DOWHILE_NOTHING()
#define LOG_DEBUGL(logger, message) DOWHILE_NOTHING()
#define LOG_INFOL(logger, message) DOWHILE_NOTHING()
#define LOG_WARNL(logger, message) DOWHILE_NOTHING()
#define LOG_ERRORL(logger, message) DOWHILE_NOTHING()
#define LOG_FATALL(logger, message) DOWHILE_NOTHING()

#define IMPLEMENT_STATIC_LOGGER(logger_name) DOWHILE_NOTHING()
#define DECLARE_GET_LOGGER(logger_name)
#define DECLARE_GLOBAL_GET_LOGGER(logger_name)

#define LOG_TRACE(message) DOWHILE_NOTHING()
#define LOG_DEBUG(message) DOWHILE_NOTHING()
#define LOG_INFO(message) DOWHILE_NOTHING()
#define LOG_WARN(message) DOWHILE_NOTHING()
#define LOG_ERROR(message) DOWHILE_NOTHING()
#define LOG_FATAL(message) DOWHILE_NOTHING()

#define LOG_AUTO_TRACEL(logger, message) DOWHILE_NOTHING()
#define LOG_AUTO_TRACE() DOWHILE_NOTHING()

#define LOG_TRACEF(text, ...) DOWHILE_NOTHING()
#define LOG_DEBUGF(text, ...) DOWHILE_NOTHING()
#define LOG_INFOF(text, ...) DOWHILE_NOTHING()
#define LOG_WARNF(text, ...) DOWHILE_NOTHING()
#define LOG_ERRORF(text, ...) DOWHILE_NOTHING()
#define LOG_FATALF(text, ...) DOWHILE_NOTHING()

#define LOG_AUTO_NDC(msg) DOWHILE_NOTHING()

#else // DISABLE_LOGGER

// Strip off log lines lower or equal DEBUG in DEBUG mode
#if defined(CUT_OFF_DEBUG_LOG)
#define LOG4CPLUS_DISABLE_DEBUG
#endif // CUT_OFF_DEBUG_LOG

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include <bits/stringfwd.h>
#include <iosfwd>
#include <ostream>
#include <string>

#include <fmt/format.h>

#define IMPL_LOGGER_CLASS_TYPE_ log4cplus::Logger
#define IMPL_LOGGER_NAMESPACE_ fservice

namespace fservice {

class LogManager {
 public:
  explicit LogManager(std::string const& configFilePath);

  explicit LogManager(std::istream& logConfig);

  ~LogManager();

  static void shutdown();

  LogManager(LogManager const&) = delete;
  LogManager(LogManager const&&) = delete;
  LogManager& operator=(LogManager const&) = delete;
  LogManager& operator=(LogManager const&&) = delete;
};

class NDCWrapper {
 public:
  explicit NDCWrapper(std::string const& msg);
  ~NDCWrapper();
};

} // namespace fservice

#define INIT_LOGGER(logConfig) \
  IMPL_LOGGER_NAMESPACE_::LogManager logManager__(logConfig)
#define SHUTDOWN_LOGGER() IMPL_LOGGER_NAMESPACE_::LogManager::shutdown();

#define LOG_TRACEL(logger, message) \
  LOG4CPLUS_TRACE(logger, LOG4CPLUS_TEXT(message))
#define LOG_DEBUGL(logger, message) \
  LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT(message))
#define LOG_INFOL(logger, message) \
  LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT(message))
#define LOG_WARNL(logger, message) \
  LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT(message))
#define LOG_ERRORL(logger, message) \
  LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT(message))
#define LOG_FATALL(logger, message) \
  LOG4CPLUS_FATAL(logger, LOG4CPLUS_TEXT(message))

#define IMPLEMENT_STATIC_LOGGER(loggerName) \
  static auto logger =                      \
      IMPL_LOGGER_CLASS_TYPE_::getInstance(LOG4CPLUS_TEXT(loggerName))

#define DECLARE_GET_LOGGER(loggerName)                          \
  [[gnu::unused]] static IMPL_LOGGER_CLASS_TYPE_& getLogger() { \
    IMPLEMENT_STATIC_LOGGER(loggerName);                        \
    return logger;                                              \
  }

#define DECLARE_GLOBAL_GET_LOGGER(loggerName) \
  namespace {                                 \
  DECLARE_GET_LOGGER(loggerName)              \
  } // namespace

#define LOG_TRACE(message) LOG_TRACEL(getLogger(), message)
#define LOG_DEBUG(message) LOG_DEBUGL(getLogger(), message)
#define LOG_INFO(message) LOG_INFOL(getLogger(), message)
#define LOG_WARN(message) LOG_WARNL(getLogger(), message)
#define LOG_ERROR(message) LOG_ERRORL(getLogger(), message)
#define LOG_FATAL(message) LOG_FATALL(getLogger(), message)

#define LOG_AUTO_TRACEL(logger, message) LOG4CPLUS_TRACE_METHOD(logger, message)
#define LOG_AUTO_TRACE() LOG_AUTO_TRACEL(getLogger(), LOG4CPLUS_TEXT(__func__))

#define LOG_TRACEF(text, ...) \
  LOG_TRACEL(getLogger(), fmt::format(text, __VA_ARGS__))
#define LOG_DEBUGF(text, ...) \
  LOG_DEBUGL(getLogger(), fmt::format(text, __VA_ARGS__))
#define LOG_INFOF(text, ...) \
  LOG_INFOL(getLogger(), fmt::format(text, __VA_ARGS__))
#define LOG_WARNF(text, ...) \
  LOG_WARNL(getLogger(), fmt::format(text, __VA_ARGS__))
#define LOG_ERRORF(text, ...) \
  LOG_ERRORL(getLogger(), fmt::format(text, __VA_ARGS__))
#define LOG_FATALF(text, ...) \
  LOG_FATALL(getLogger(), fmt::format(text, __VA_ARGS__))

#define LOG_AUTO_NDC(msg) IMPL_LOGGER_NAMESPACE_::NDCWrapper ndc_wrapper__(msg)

#endif
