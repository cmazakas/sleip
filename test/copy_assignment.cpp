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
    auto a = sleip::dynamic_array<int>();
    auto b = sleip::dynamic_array<int>{1, 2, 3};

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);

    a = b;

    BOOST_TEST_ALL_EQ(a.begin(), a.end(), b.begin(), b.end());
  }

  // non-empty case
  {
    auto a = sleip::dynamic_array<int>{1, 2, 3};
    auto b = sleip::dynamic_array<int>{4, 5, 6};

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_NE(a.data()[0], b.data()[0]);

    b = a;

    BOOST_TEST_ALL_EQ(b.begin(), b.end(), a.begin(), a.end());
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
    auto a = sleip::dynamic_array<throwing>();
    auto b = sleip::dynamic_array<throwing>(3);

    for (auto& t : b) { t.x = -1; }

    BOOST_TEST(a.get_allocator() == b.get_allocator());
    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);
  }

  // non-empty case
  {
    auto a = sleip::dynamic_array<throwing>(4);
    auto b = sleip::dynamic_array<throwing>(6);

    BOOST_TEST(a.get_allocator() == b.get_allocator());

    BOOST_TEST_EQ(a.size(), 4);
    for (auto& t : a) { t.x = -1; }

    BOOST_TEST_THROWS((b = a), int);

    BOOST_TEST_EQ(a.size(), 4);
    BOOST_TEST(std::all_of(a.begin(), a.end(), [](auto const& t) { return t.x == -1; }));
  }
}
#endif

void
test_copy_assignment_non_equal_allocators()
{
  // empty
  {
    auto mem           = std::array<std::byte, 4096>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<int>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<int>();

    BOOST_TEST(alloc1 != alloc2);

    auto a = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(alloc1);
    auto b = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(1024, 1, alloc2);

    BOOST_TEST_EQ(a.size(), 0);
    BOOST_TEST_EQ(a.data(), nullptr);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    a = b;

    BOOST_TEST_ALL_EQ(a.begin(), a.end(), b.begin(), b.end());
    BOOST_TEST_EQ(buff_resource.remaining_storage(), 0);
  }

  // non-empty
  {
    auto mem           = std::array<std::byte, 2 * 4096>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<int>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<int>();

    BOOST_TEST(alloc1 != alloc2);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    auto const size = 1024;

    auto a = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(size, -1, alloc1);
    auto b = sleip::dynamic_array<int, pmr::polymorphic_allocator<int>>(size / 2, 1, alloc2);

    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() - size * sizeof(int));

    a = b;

    BOOST_TEST_ALL_EQ(a.begin(), a.end(), b.begin(), b.end());
    BOOST_TEST_EQ(buff_resource.remaining_storage(),
                  mem.size() - ((size + size / 2) * sizeof(int)));
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
    auto mem           = std::array<std::byte, 4096>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<throwing>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<throwing>();

    BOOST_TEST(alloc1 != alloc2);

    auto const size = std::size_t{1024};

    auto a = sleip::dynamic_array<throwing, pmr::polymorphic_allocator<throwing>>(alloc1);
    auto b = sleip::dynamic_array<throwing, pmr::polymorphic_allocator<throwing>>(size, alloc2);

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
    auto mem           = std::array<std::byte, 2 * 4096>{};
    auto buff_resource = pmr::monotonic_buffer_resource(mem.data(), mem.size());

    auto alloc1 = pmr::polymorphic_allocator<throwing>(&buff_resource);
    auto alloc2 = pmr::polymorphic_allocator<throwing>();

    BOOST_TEST(alloc1 != alloc2);
    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size());

    auto const size = std::size_t{1024};

    auto a = sleip::dynamic_array<throwing, pmr::polymorphic_allocator<throwing>>(size, alloc1);
    auto b = sleip::dynamic_array<throwing, pmr::polymorphic_allocator<throwing>>(size / 2, alloc2);

    BOOST_TEST_EQ(buff_resource.remaining_storage(), mem.size() - size * sizeof(throwing));

    BOOST_TEST_THROWS((a = b), int);

    BOOST_TEST_EQ(a.size(), size);

    // TODO: does this break the strong exception guarantee?
    //
    BOOST_TEST_EQ(buff_resource.remaining_storage(),
                  mem.size() - (size + size / 2) * sizeof(throwing));
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
