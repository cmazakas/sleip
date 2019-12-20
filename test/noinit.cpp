#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <array>
#include <cstddef>
#include <numeric>

namespace pmr = boost::container::pmr;

void
test_default_allocator()
{
  auto       a = sleip::dynamic_array<int>(sleip::default_init, 5);
  auto const e = std::initializer_list<int>{1, 2, 3, 4, 5};

  // comment this out and test with valgrind to check effects of no default initialization
  // valgrind should have a hard error when `std::iota` is commented out
  //
  std::iota(a.begin(), a.end(), 1);

  BOOST_TEST_ALL_EQ(a.begin(), a.end(), e.begin(), e.end());
}

void
test_polymorphic_allocator()
{
  // `bytes` _must_ be unitialized for valgrind to catch this
  //
  std::array<std::byte, 4096> bytes;

  auto buff  = pmr::monotonic_buffer_resource(bytes.data(), bytes.size());
  auto alloc = pmr::polymorphic_allocator<int>(&buff);

  auto a =
    sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(sleip::default_init, 5, alloc);

  auto const e = std::initializer_list<int>{1, 2, 3, 4, 5};

  // comment this out and test with valgrind to check effects of no default initialization
  // valgrind should have a hard error when `std::iota` is commented out
  //
  std::iota(a.begin(), a.end(), 1);

  BOOST_TEST_ALL_EQ(a.begin(), a.end(), e.begin(), e.end());
}

int
main()
{
  test_default_allocator();
  test_polymorphic_allocator();

  return boost::report_errors();
}
