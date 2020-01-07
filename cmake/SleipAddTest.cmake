set(THREADS_PREFER_PTHREAD_FLAG ON CACHE BOOL "")

find_package(Boost 1.71 REQUIRED container)
find_package(Threads REQUIRED)

function(sleip_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")

  target_link_libraries(${test_name} PRIVATE dynamic_array Boost::container Threads::Threads)
  set_target_properties(${test_name} PROPERTIES FOLDER "Test")

  if (MSVC)
    target_link_libraries(${test_name} PRIVATE Boost::disable_autolinking)
  endif()

  if (UNIX AND NOT APPLE)
    target_link_libraries(${test_name} PRIVATE rt)
  endif()

  add_test(${test_name} ${test_name})
endfunction()
