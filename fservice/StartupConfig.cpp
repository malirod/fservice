// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#include <fservice/GeneralError.h>
#include <fservice/PathUtil.h>
#include <fservice/StartupConfig.h>
#include <fservice/Version.h>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <thread>

namespace fservice {

outcome::result<StartupConfig> processCmdArgs(int argc, char** argv) {
  namespace po = boost::program_options;

  auto const printHelp = [](auto const& options) { std::cout << options; };
  auto const printVersion = []() {
    std::cout << "Version: " << getVersion() << "\n";
  };
  auto const printError = [](auto const& error) {
    std::cerr << "Error: " << error.what() << "\n";
  };

  std::string configFilePath;
  po::options_description generalOptions("General options");
  generalOptions.add_options()("help,h", "Print help")(
      "version,v", "Print application version")(
      "config,c",
      po::value(&configFilePath)
          ->default_value(
              fservice::getExePath(argv[0]).replace_extension(".cfg")),
      "Configuration file path");

  po::options_description serverOptions("Server options");
  std::string ip;
  std::uint32_t port;
  std::uint32_t threads;
  serverOptions.add_options()(
      "ip,i", po::value(&ip)->default_value("127.0.0.1"), "Set ip to listen")(
      "port,p", po::value(&port)->default_value(12001), "Set port to listen")(
      "threads,t",
      po::value(&threads)->default_value(std::thread::hardware_concurrency()),
      "Number of threads to listen on. Numbers <= 0. Will use the number of "
      "cores on this machine.");

  po::options_description allOptions("Allowed options");
  allOptions.add(generalOptions).add(serverOptions);

  po::variables_map vm;
  try {
    po::parsed_options parsed_options = po::command_line_parser(argc, argv)
                                            .options(allOptions)
                                            .allow_unregistered()
                                            .run();

    po::store(parsed_options, vm);
    po::notify(vm);

    std::ifstream configFileStream(configFilePath.c_str());
    if (!configFileStream) {
      std::cerr << "Cannot open configuration file : " << configFilePath
                << "\n";
    } else {
      po::store(po::parse_config_file(configFileStream, serverOptions), vm);
      po::notify(vm);
    }
  } catch (po::error const& error) {
    printError(error);
    printHelp(allOptions);
    return GeneralError::WrongStartupParams;
  }

  if (vm.count("help") != 0u) {
    printHelp(allOptions);
    return GeneralError::Interrupted;
  }

  if (vm.count("version") != 0u) {
    printVersion();
    return GeneralError::Interrupted;
  }

  try {
    bool const allowNameLookup = true;
    return StartupConfig{folly::SocketAddress(ip, port, allowNameLookup),
                         threads};
  } catch (std::exception const& error) {
    printError(error);
    printHelp(allOptions);
    return GeneralError::WrongStartupParams;
  }
}

} // namespace  fservice