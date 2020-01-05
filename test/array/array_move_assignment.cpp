#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>
#include <array>
#include <cstddef>

namespace pmr = boost::container::pmr;

void
test_move_assignment_equal_allocators()
{
  // empty
  //
  {
    auto count = 16;

    auto a = sleip::dynamic_array<int[3]>();
    auto b = sleip::dynamic_array<int[3]>(2 * count, {1, 2, 3});

    BOOST_TEST(a.get_allocator() == b.get_allocator());

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * count);
    BOOST_TEST_EQ(a.data(), old_data);

    BOOST_TEST_EQ(b.data(), nullptr);
    BOOST_TEST_EQ(b.size(), 0);
  }

  // non-empty
  //
  {
    auto count = 16;

    auto a = sleip::dynamic_array<std::vector<int>[3]>(count);
    auto b = sleip::dynamic_array<std::vector<int>[3]>(2 * count);

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * count);
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
    auto const size = 16;

    auto mem  = std::array<std::byte, size * sizeof(int[3])>{};
    auto buff = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto buff_alloc    = pmr::polymorphic_allocator<int>(&buff);
    auto default_alloc = pmr::polymorphic_allocator<int>();

    auto a = sleip::dynamic_array<int[3], pmr::polymorphic_allocator<int[3]>>(buff_alloc);
    auto b =
      sleip::dynamic_array<int[3], pmr::polymorphic_allocator<int[3]>>(2 * size, default_alloc);

    BOOST_TEST(a.get_allocator() != b.get_allocator());

    auto old_data = b.data();

    a = std::move(b);

    BOOST_TEST_EQ(a.size(), 2 * size);
    BOOST_TEST_NE(a.data(), old_data);

    BOOST_TEST_EQ(b.data(), old_data);
    BOOST_TEST_EQ(b.size(), 2 * size);
  }

  // non-empty
  //
  {
    auto const size = 4;

    auto mem  = std::array<std::byte, size * sizeof(std::unique_ptr<int>[3])>{};
    auto buff = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto buff_alloc    = pmr::polymorphic_allocator<std::unique_ptr<int>[3]>(&buff);
    auto default_alloc = pmr::polymorphic_allocator<std::unique_ptr<int>[3]>();

    auto a =
      sleip::dynamic_array<std::unique_ptr<int>[3],
                           pmr::polymorphic_allocator<std::unique_ptr<int>[3]>>(size, buff_alloc);

    auto b = sleip::dynamic_array<std::unique_ptr<int>[3],
                                  pmr::polymorphic_allocator<std::unique_ptr<int>[3]>>(
      2 * size, default_alloc);

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
