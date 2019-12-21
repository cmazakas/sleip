#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <array>
#include <cstddef>
#include <numeric>
#include <cstring>
#include <algorithm>

namespace pmr = boost::container::pmr;

// write a sentinel byte value to a memory region that we use for storage for our dynamic array
// to goal is to prove that our noinit construction of the `dynamic_array` does not alter its
// underlying memory region
//
void
test_noinit()
{
  auto const count    = 1024;
  auto const sentinel = std::byte{123};

  // prove the noinit case before proving the init case
  //
  {
    auto buf = std::array<std::byte, count * sizeof(int)>{};
    buf.fill(sentinel);

    auto const expected_bytes = buf;

    auto mem_resouce = pmr::monotonic_buffer_resource(buf.data(), buf.size());
    auto alloc       = pmr::polymorphic_allocator<int>(&mem_resouce);

    auto a =
      sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(count, sleip::noinit, alloc);

    BOOST_TEST_EQ(a.size(), count);
    BOOST_TEST(buf == expected_bytes);
  }

  // prove that initialization _does_ alter the underlying byte sequence
  //
  {
    auto buf = std::array<std::byte, count * sizeof(int)>{};
    buf.fill(sentinel);

    auto const expected_bytes = buf;

    auto mem_resouce = pmr::monotonic_buffer_resource(buf.data(), buf.size());
    auto alloc       = pmr::polymorphic_allocator<int>(&mem_resouce);

    auto a = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(count, -1, alloc);

    BOOST_TEST_EQ(a.size(), count);
    BOOST_TEST(buf != expected_bytes);
  }
}

int
main()
{
  test_noinit();

  return boost::report_errors();
}
