#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>
#include <iostream>

namespace pmr = boost::container::pmr;

void
test_move_assignment_equal_allocators()
{
  // empty
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

int
main()
{
  test_move_assignment_equal_allocators();
  return boost::report_errors();
}
