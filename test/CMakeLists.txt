function(sleip_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE dynamic_array)
  add_test(${test_name} ${test_name})
endfunction()

sleip_add_test(container_traits)
sleip_add_test(constructors)
