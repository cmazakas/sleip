cmake_minimum_required(VERSION 3.13)

project(
  dynamic_array

  LANGUAGES
    CXX

  VERSION
    1.0.0
)

set(CMAKE_CXX_STANDARD "17" CACHE STRING "Required C++ standard")
if(CMAKE_CXX_STANDARD LESS 17)
  message(FATAL_ERROR "Minimum C++17 version not met!")
endif()

find_package(Boost 1.71 REQUIRED)

include(GNUInstallDirs)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(dynamic_array INTERFACE)
target_include_directories(
  dynamic_array
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/sleip-${PROJECT_VERSION}>
)

target_link_libraries(dynamic_array INTERFACE Boost::headers)

include(CTest)
if (BUILD_TESTING)
  include("cmake/SleipAddTest.cmake")
  add_subdirectory(test)
endif()

option(
  SLEIP_ADD_SUBDIRECTORY
  "set(SLEIP_ADD_SUBDIRECTORY ON CACHE BOOL \"\") to disable installation rules allowing Sleip to support add_subdirectory"
  OFF
)

if (SLEIP_ADD_SUBDIRECTORY)
  add_library(Sleip::dynamic_array ALIAS dynamic_array)
endif()

if (NOT SLEIP_ADD_SUBDIRECTORY)
  install(
    TARGETS
      dynamic_array

    EXPORT
      sleip-targets

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    EXPORT
      sleip-targets

    NAMESPACE Sleip::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleip-${PROJECT_VERSION}
  )

  install(
    DIRECTORY
      include/

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sleip-${PROJECT_VERSION}
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
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleip-${PROJECT_VERSION}
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/sleip-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/sleip-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleip-${PROJECT_VERSION}
  )
endif()
