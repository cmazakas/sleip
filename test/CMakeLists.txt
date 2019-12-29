set(THREADS_PREFER_PTHREAD_FLAG ON CACHE BOOL "")

find_package(Boost 1.71 REQUIRED container)
find_package(Threads REQUIRED)

function(sleip_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")

  target_link_libraries(${test_name} PRIVATE dynamic_array)
  if (MSVC)
    target_link_libraries(${test_name} PUBLIC Boost::disable_autolinking)
  endif()
  set_target_properties(${test_name} PROPERTIES FOLDER "Test")

  add_test(${test_name} ${test_name})
endfunction()

sleip_add_test(container_traits)
sleip_add_test(move_if_noexcept_adaptor)
sleip_add_test(constructors)
target_link_libraries(constructors PRIVATE Boost::container)

sleip_add_test(copy_assignment)
target_link_libraries(copy_assignment PRIVATE Boost::container)

sleip_add_test(move_assignment)
target_link_libraries(move_assignment PRIVATE Boost::container)

sleip_add_test(initializer_list_assignment)
sleip_add_test(element_access)
sleip_add_test(reverse_iterators)
sleip_add_test(capacity)
sleip_add_test(operations)

sleip_add_test(fancy_pointer)
target_link_libraries(fancy_pointer PRIVATE Threads::Threads)
if (UNIX AND NOT APPLE)
  target_link_libraries(fancy_pointer PRIVATE rt)
endif()

sleip_add_test(noinit)
target_link_libraries(noinit PRIVATE Boost::container)

sleip_add_test(comparison)
