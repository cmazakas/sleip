#include <sleip/dynamic_array.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <cstddef>
#include <array>

namespace pmr = boost::container::pmr;

void
test_copy_assignment_equal_allocators()
{
  // empty case
  {
    int value[3] = {1, 2, 3};

    auto a = sleip::dynamic_array<int[3]>();
    auto b = sleip::dynamic_array<int[3]>(3, value);

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);

    a = b;

    BOOST_TEST_EQ(a.size(), 3);
    for (auto const& arr : a) {
      BOOST_TEST(std::equal(std::begin(arr), std::end(arr), std::begin(value), std::end(value)));
    }
  }

  // non-empty case
  {
    int value[3] = {1, 2, 3};

    auto a = sleip::dynamic_array<int[3]>(3, value);
    auto b = sleip::dynamic_array<int[3]>(3, {4, 5, 6});

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_NE(a[0][0], b[0][0]);

    for (auto const& arr : b) {
      BOOST_TEST(!std::equal(std::begin(arr), std::end(arr), std::begin(value), std::end(value)));
    }

    b = a;

    for (auto const& arr : b) {
      BOOST_TEST(std::equal(std::begin(arr), std::end(arr), std::begin(value), std::end(value)));
    }
  }
}

#ifdef BOOST_NO_EXCEPTIONS

void
test_copy_assignment_equal_allocators_throwing()
{
}

#else

void
test_copy_assignment_equal_allocators_throwing()
{
  struct throwing
  {
    int x = 0;

    throwing() = default;
    throwing(throwing const&) { throw 42; };
    throwing(throwing&&) = default;

    throwing(int x_)
      : x{x_}
    {
    }
  };

  // empty case
  {
    auto a = sleip::dynamic_array<throwing[3]>();
    auto b = sleip::dynamic_array<throwing[3]>(3);

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);
  }

  // non-empty case
  {
    auto a = sleip::dynamic_array<throwing[3]>(4);
    auto b = sleip::dynamic_array<throwing[3]>(6);

    BOOST_TEST(a.get_allocator() == b.get_allocator());

    BOOST_TEST_EQ(a.size(), 4);
    for (auto& t : a) { t[0].x = t[1].x = t[2].x = -1; }

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), 4);
    BOOST_TEST(std::all_of(a.begin(), a.end(), [](auto const& t) {
      return std::all_of(std::begin(t), std::end(t), [](auto const& v) { return v.x == -1; });
    }));
  }
}
#endif

void
test_copy_assignment_non_equal_allocators()
{
  // empty
  {
    auto mem           = std::array<std::byte, 3 * 1024 * sizeof(int)>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<int[3]>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<int[3]>();

    BOOST_TEST(alloc1 != alloc2);

    int value[3] = {1, 2, 3};

    auto a = sleip::dynamic_array<int[3], pmr::polymorphic_allocator<int[3]>>(alloc1);
    auto b = sleip::dynamic_array<int[3], pmr::polymorphic_allocator<int[3]>>(1024, value, alloc2);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    a = b;

    BOOST_TEST_EQ(buff_resource.remaining_storage(), 0);
    BOOST_TEST_EQ(a.size(), b.size());
    for (auto const& arr : a) {
      BOOST_TEST_ALL_EQ(std::begin(arr), std::end(arr), std::begin(value), std::end(value));
    }
  }

  // non-empty
  {
    auto mem           = std::array<std::byte, 2 * 3 * 1024 * sizeof(int)>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<int[1][3]>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<int[1][3]>();

    BOOST_TEST(alloc1 != alloc2);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    auto const size = 1024;

    int value[1][3] = {{1, 2, 3}};

    auto a = sleip::dynamic_array<int[1][3], pmr::polymorphic_allocator<int[1][3]>>(size, alloc1);
    auto b = sleip::dynamic_array<int[1][3], pmr::polymorphic_allocator<int[1][3]>>(size / 2, value,
                                                                                    alloc2);

    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() / 2);

    a = b;

    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() / 4);
    BOOST_TEST_EQ(a.size(), b.size());

    for (auto const& arr : a) {
      auto const begin1 = boost::first_scalar(arr);
      auto const begin2 = boost::first_scalar(value);

      BOOST_TEST_ALL_EQ(begin1, begin1 + 3, begin2, begin2 + 3);
    }
  }
}

#ifdef BOOST_NO_EXCEPTIONS

void
test_copy_assignment_non_equal_allocators_throwing()
{
}

#else

void
test_copy_assignment_non_equal_allocators_throwing()
{
  struct throwing
  {
    int x = 0;

    throwing() = default;
    throwing(throwing const&) { throw 42; };

    throwing(throwing&&) = delete;

    auto
    operator=(throwing const&) -> throwing&
    {
      throw 42;
      return *this;
    }

    auto
    operator=(throwing&&) noexcept -> throwing& = delete;
  };

  // empty
  {
    auto mem           = std::array<std::byte, 3 * 1024 * sizeof(throwing)>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<throwing[3]>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<throwing[3]>();

    BOOST_TEST(alloc1 != alloc2);

    auto const size = 1024;

    auto a = sleip::dynamic_array<throwing[3], pmr::polymorphic_allocator<throwing[3]>>(alloc1);
    auto b =
      sleip::dynamic_array<throwing[3], pmr::polymorphic_allocator<throwing[3]>>(size, alloc2);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);

    // TODO: does this break the strong exception guarantee?
    // technically, deallocate is called but in the case of a monotonic_buffer_resource, deallocate
    // is a no-op so even though we release the temporary storage we acquire, its effects are felt
    // by the user
    //
    BOOST_TEST_EQ(buff_resource.remaining_storage(), 0);
  }

  // non-empty
  {
    auto const size = 1024;

    auto mem           = std::array<std::byte, 2 * size * sizeof(throwing[3])>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<throwing[3]>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<throwing[3]>();

    BOOST_TEST(alloc1 != alloc2);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    auto a =
      sleip::dynamic_array<throwing[3], pmr::polymorphic_allocator<throwing[3]>>(size, alloc1);

    auto b =
      sleip::dynamic_array<throwing[3], pmr::polymorphic_allocator<throwing[3]>>(size / 2, alloc2);

    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() / 2);

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), size);

    // TODO: does this break the strong exception guarantee?
    //
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() / 4);
  }
}

#endif

int
main()
{
  test_copy_assignment_equal_allocators();
  test_copy_assignment_equal_allocators_throwing();
  test_copy_assignment_non_equal_allocators();
  test_copy_assignment_non_equal_allocators_throwing();

  return boost::report_errors();
}
