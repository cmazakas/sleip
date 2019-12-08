find_package(Boost 1.71 REQUIRED container)

function(sleip_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE dynamic_array)
  add_test(${test_name} ${test_name})
endfunction()

sleip_add_test(container_traits)
sleip_add_test(constructors)

sleip_add_test(copy_assignment)
target_link_libraries(copy_assignment PRIVATE Boost::container)
