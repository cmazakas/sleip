# Dynamic Array

`sleip::dynamic_array` is An [AllocatorAwareContainer](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer)
that represents a dynamically-allocated fixed-size buffer. It is a combination of the constructors
for [`std::vector`](https://en.cppreference.com/w/cpp/container/vector)
with the member functions from [`std::array`](https://en.cppreference.com/w/cpp/container/array).

Unlike many containers in the STL, `sleip::dynamic_array` supports array objects and has a `noinit`
constructor overload that enables [default initialization](https://en.cppreference.com/w/cpp/language/default_initialization)
of its underlying elements.

The `dynamic_array` is useful wherever a user may need a managed dynamic allocation where they
won't need resizing or `push_back`-like functionality.
Because it supports [Allocators](https://en.cppreference.com/w/cpp/named_req/Allocator) and can
default-initialize its elements, `dynamic_array` is suitable for performance-oriented applications.

## Example

```c++
#include <sleip/dynamic_array.hpp>

auto buff = sleip::dynamic_array<char>(480 * 640 * 3, sleip::noinit);
auto file = std::fstream("file.dat", std::ios::in | std::ios::binary);
file.read(buff.data(), buff.size());
```

## Using

### Via vcpkg

`dynamic_array` is available as a [vcpkg](https://github.com/Microsoft/vcpkg) package from the port
directory [here](https://github.com/LeonineKing1199/sleip-port).

```bash
# navigate to the vcpkg root on your system
cd ~/vcpkg

git clone https://github.com/LeonineKing1199/sleip-port.git
./vcpkg install sleip --overlay-ports=sleip-port

# The package sleip:x64-linux provides CMake targets:
#
#     find_package(sleip 1.0.0 CONFIG REQUIRED)
#     target_link_libraries(main PRIVATE Sleip::dynamic_array)
```

### Via CMake Install

`dynamic_array` supports installation with CMake.

```bash
cd ~
git clone https://github.com/LeonineKing1199/sleip.git
cd sleip
mkdir build
cd build
cmake \
  -DBoost_DIR=/home/exbigboss/boosts/71/lib/cmake/Boost-1.71.0 \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=../install \
  ..
cmake --install .

tree ../install/
# ../install/
# ├── include
# │   └── sleip-1.0.0
# │       └── sleip
# │           ├── dynamic_array.hpp
# │           └── dynamic_array_fwd.hpp
# └── lib
#     └── cmake
#         └── sleip-1.0.0
#             ├── sleip-config-version.cmake
#             ├── sleip-config.cmake
#             └── sleip-targets.cmake
```

Now `find_package(Sleip 1.0.0)` will work when the `sleip_DIR` is set to:
`<INSTALL_PREFIX>/lib/cmake/sleip-1.0.0`.

Using our above example:

```bash
cmake -Dsleip_DIR=~/sleip/install/lib/cmake/sleip-1.0.0 ..
```

The following will now work in CMake:

```bash
find_package(sleip 1.0.0 CONFIG REQUIRED)
target_link_libraries(main PRIVATE Sleip::dynamic_array)
```

### Via CMake add_subdirectory

`dynamic_array` can be used directly as a CMake sub-project.

```bash
cd ~/project
git clone https://github.com/LeonineKing1199/sleip.git
```

Then in your project's CMakeLists.txt,

```cmake
cmake_minimum_required(VERSION 3.13)

project(sleip-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SLEIP_ADD_SUBDIRECTORY ON CACHE BOOL "")

add_subdirectory(sleip)
add_executable(test main.cpp)

target_link_libraries(test PRIVATE Sleip::dynamic_array)
```

By default, this will build the tests for `dynamic_array` alongside the consuming project's.
