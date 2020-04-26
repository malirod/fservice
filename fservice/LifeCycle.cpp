// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/EngineLauncher.h>
#include <fservice/GeneralError.h>
#include <fservice/Logger.h>
#include <fservice/PathUtil.h>
#include <fservice/StartupConfig.h>

#include <folly/init/Init.h>

#include <cassert>

DECLARE_GLOBAL_GET_LOGGER("Main")

/**
 * Main entry point of the application.
 * @param argc Count of command line arguments.
 * @param argv Command line arguments.
 * @return Error code.
 */
int main(int argc, char** argv) {
  assert(argc > 0);
  assert(argv[0] != nullptr);
  folly::init(&argc, &argv, true); // We have to init folly internals (logger,
                                   // singleton etc)

  using fservice::EngineLauncher;
  using fservice::GeneralError;
  using fservice::getExePath;
  using fservice::processCmdArgs;
  using fservice::StartupConfig;

  INIT_LOGGER(getExePath(argv[0]).replace_filename("logger.cfg"));

  try {
    auto const startupConfigOrError = processCmdArgs(argc, argv);
    if (!startupConfigOrError) {
      if (startupConfigOrError.error() == GeneralError::Interrupted) {
        return make_error_code(GeneralError::Success).value();
      } else if (startupConfigOrError.error() ==
                 GeneralError::WrongStartupParams) {
        LOG_DEBUG("Wrong startup paramer(s)");
        return startupConfigOrError.error().value();
      } else {
        LOG_ERRORF("Unhandled command line error: {}",
                   startupConfigOrError.error().message());
        return startupConfigOrError.error().value();
      }
    }
    auto const startupConfig = startupConfigOrError.value();
    LOG_INFO("Starting server");
    EngineLauncher engineLauncher(std::move(startupConfig));

    const auto errorCode = engineLauncher.run();
    LOG_INFOF("Server has finished with exit code: '{}'", errorCode.message());
    return errorCode.value();
  } catch (std::exception const& error) {
    LOG_FATALF("Exception has occurred: {}", error.what());
  } catch (...) {
    LOG_FATAL("Unknown exception has occurred");
  }
  auto const errorCode = make_error_code(GeneralError::InternalError);
  LOG_FATALF("Unexpected exit: {}", errorCode.message());
  return errorCode.value();
}