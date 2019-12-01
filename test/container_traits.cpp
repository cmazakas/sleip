#include <sleip/dynamic_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::value_type, int>,
              "DynamicArray's value_type must match its template");

static_assert(
  std::is_same_v<typename sleip::dynamic_array<int, std::allocator<int>>::allocator_type,
                 std::allocator<int>>,
  "DynamicArray's allocator_type must match its template");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::size_type, std::size_t>,
              "DynamicArray's size_type should be std::size_t");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::difference_type, std::ptrdiff_t>,
              "DynamicArray's difference_type should be std::ptrdiff_t");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::reference,
                             typename sleip::dynamic_array<int>::value_type&>,
              "DynamicArray's reference should be a reference to its value type");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::const_reference,
                             typename sleip::dynamic_array<int>::value_type const&>,
              "DynamicArray's const_reference should be a const reference to its value type");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::pointer,
                             typename sleip::dynamic_array<int>::value_type*>,
              "DynamicArray's pointer should be a pointer to its value type");

static_assert(std::is_same_v<typename sleip::dynamic_array<int>::const_pointer,
                             typename sleip::dynamic_array<int>::value_type const*>,
              "DynamicArray's const_pointer should be a const pointer to its value type");

int
main()
{
  return boost::report_errors();
}
