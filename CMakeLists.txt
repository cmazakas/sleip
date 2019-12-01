cmake_minimum_required(VERSION 3.13)

project(
  dynamic_array

  LANGUAGES
    CXX
)

find_package(Boost 1.71 REQUIRED)

add_library(dynamic_array INTERFACE)
target_include_directories(dynamic_array INTERFACE include)
target_link_libraries(dynamic_array INTERFACE Boost::headers)
target_compile_features(dynamic_array INTERFACE cxx_std_17)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()
