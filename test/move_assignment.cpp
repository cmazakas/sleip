#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>
#include <array>
#include <cstddef>
#include <iostream>

namespace pmr = boost::container::pmr;

void
test_move_assignment_equal_allocators()
{
  // empty
  //
  {
    auto size = std::size_t{16};

    auto a = sleip::dynamic_array<int>();
    auto b = sleip::dynamic_array<int>(2 * size, 1);

    BOOST_TEST(a.get_allocator() == b.get_allocator());

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * size);
    BOOST_TEST_EQ(a.data(), old_data);

    BOOST_TEST_EQ(b.data(), nullptr);
    BOOST_TEST_EQ(b.size(), 0);
  }

  // non-empty
  //
  {
    auto size = std::size_t{16};

    auto a = sleip::dynamic_array<std::vector<int>>(size, std::vector{1, 2, 3});
    auto b = sleip::dynamic_array<std::vector<int>>(2 * size, std::vector{1, 2, 3});

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * size);
    BOOST_TEST_EQ(a.data(), old_data);

    BOOST_TEST_EQ(b.data(), nullptr);
    BOOST_TEST_EQ(b.size(), 0);
  }
}

void
test_move_assignment_non_equal_allocator()
{
  // empty
  //
  {
    auto mem  = std::array<std::byte, 4096>{};
    auto buff = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto buff_alloc    = pmr::polymorphic_allocator<int>(&buff);
    auto default_alloc = pmr::polymorphic_allocator<int>();

    auto size = std::size_t{16};

    auto a = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(buff_alloc);
    auto b = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(2 * size, 1, default_alloc);

    BOOST_TEST(a.get_allocator() != b.get_allocator());

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * size);

    BOOST_TEST_EQ(b.data(), old_data);
    BOOST_TEST_EQ(b.size(), 2 * size);
  }

  // non-empty
  //
  {
    auto mem  = std::array<std::byte, 4096>{};
    auto buff = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto buff_alloc    = pmr::polymorphic_allocator<std::vector<int>>(&buff);
    auto default_alloc = pmr::polymorphic_allocator<std::vector<int>>();

    auto size = std::size_t{4};

    auto a = sleip::dynamic_array<std::vector<int>, pmr::polymorphic_allocator<std::vector<int>>>(
      size, std::vector{1, 2, 3}, buff_alloc);

    auto b = sleip::dynamic_array<std::vector<int>, pmr::polymorphic_allocator<std::vector<int>>>(
      2 * size, std::vector{1, 2, 3}, default_alloc);

    BOOST_TEST(a.get_allocator() != b.get_allocator());

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * size);

    BOOST_TEST_EQ(b.data(), old_data);
    BOOST_TEST_EQ(b.size(), 2 * size);
  }
}

int
main()
{
  test_move_assignment_equal_allocators();
  test_move_assignment_non_equal_allocator();
  return boost::report_errors();
}
