# FService

Template service based on Folly.

## Platform

Ubuntu 19.10: GCC 9, Cmake 3.13, Vcpkg

C++17 Standard is used.

## Setup

Submodules are used. Initial command after clone is `git submodule update --init --recursive`.

Assuming all further commands are executed from project root.

### Setup git hook

Run `tools/install_hooks.py`

This will allow to perform some code checks locally before posting changes to server.

### Dependencies

Libs: Folly, Catch2, Fmt, Log4cplus, trompeloeil

Project uses [Vcpkg Package Manager](https://github.com/microsoft/vcpkg)

Install dependencies with

```bash
vcpkg install folly
vcpkg install catch2
vcpkg install fmt
vcpkg install log4cplus
vcpkg install trompeloeil
```

CMake will try to automatically setup dependencies.

CMake will automatically check required dependencies and setup them taking into account current compiler (clang or gcc).

#### Vcpkg build with dependencies with different compiler.

By defualt Vcpkg uses gcc to build dependencies. To build with other compiler set corresponding env vars.

E.g. to build fmt with clang use `CC=clang CXX=clang++ vcpkg install fmt`

### VS Code integration

Edit setting.json (Setting -> Cmake: Configure setting)

Add

```
"cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "<vcpkg root>/scripts/buildsystems/vcpkg.cmake"
}
```

### Draw deps with cmake

```bash
cmake --graphviz=graph .. && dot graph -T png -o graph.png
```

## Install doxygen

`sudo apt install -y doxygen graphviz`

## Build

### Build commands

To build release version with gcc run the following command

`mkdir build-gcc-release && cd build-gcc-release && CXX=g++ cmake -DCMAKE_BUILD_TYPE=Release -GNinja .. && ninja`

### Build with sanitizers

You can enable sanitizers with `SANITIZE_ADDRESS`, `SANITIZE_MEMORY`, `SANITIZE_THREAD` or `SANITIZE_UNDEFINED` options in your CMake configuration. You can do this by passing e.g. `-DSANITIZE_ADDRESS=On` in your command line.

## Run

Run from build directory

`ctest`

or

`./build/testrunner`

## Coverage report

To enable coverage support in general, you have to enable `ENABLE_COVERAGE` option in your CMake configuration. You can do this by passing `-DENABLE_COVERAGE=On` on your command line or with your graphical interface.

If coverage is supported by your compiler, the specified targets will be build with coverage support. If your compiler has no coverage capabilities (I assume Intel compiler doesn't) you'll get a warning but CMake will continue processing and coverage will simply just be ignored.

Collect coverage in Debug mode.

### Sample commands to get coverage html report

```bash
CXX=g++ cmake -DENABLE_COVERAGE=On -DCMAKE_BUILD_TYPE=Debug -GNinja ..
ninja
ctest
ninja lcov-capture
ninja lcov-genhtml
xdg-open lcov/html/selected_targets/index.html

```

## Documentation

Code contains doxygen. To generate html documentation run command `ninja doc`. Output will be in `<build dir>\doc\html
