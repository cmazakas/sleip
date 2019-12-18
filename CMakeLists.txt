cmake_minimum_required(VERSION 3.13)

project(
  dynamic_array

  LANGUAGES
    CXX

  VERSION
    1.0.0
)

find_package(Boost 1.71 REQUIRED)

add_library(dynamic_array INTERFACE)
target_include_directories(
  dynamic_array
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/sleip-${PROJECT_VERSION}>
)

target_link_libraries(dynamic_array INTERFACE Boost::headers)
target_compile_features(dynamic_array INTERFACE cxx_std_17)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()

install(
  TARGETS
    dynamic_array

  EXPORT
    sleip-targets

  LIBRARY DESTINATION lib/
  ARCHIVE DESTINATION lib/
  RUNTIME DESTINATION bin/
)

install(
  EXPORT
    sleip-targets

  NAMESPACE Sleip::
  DESTINATION lib/cmake/sleip-${PROJECT_VERSION}
)

install(
  DIRECTORY
    include/

  DESTINATION include/sleip-${PROJECT_VERSION}
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/sleip-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/sleip-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sleip-config.cmake
  INSTALL_DESTINATION lib/cmake/sleip-${PROJECT_VERSION}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sleip-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sleip-config-version.cmake
  DESTINATION lib/cmake/sleip-${PROJECT_VERSION}
)
