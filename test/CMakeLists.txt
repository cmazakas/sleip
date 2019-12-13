find_package(Boost 1.71 REQUIRED container date_time)
find_package(Threads REQUIRED)

function(sleip_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE dynamic_array)
  add_test(${test_name} ${test_name})
endfunction()

sleip_add_test(container_traits)
sleip_add_test(constructors)

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
if (UNIX)
  target_link_libraries(fancy_pointer PRIVATE Threads::Threads rt)
endif()
