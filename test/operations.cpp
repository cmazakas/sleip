#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>
#include <utility>
#include <type_traits>

using std::swap;

static_assert(std::is_nothrow_swappable_v<sleip::dynamic_array<int>>,
              "DynamicArray fails to meet requirements of Swappable");

void
test_fill()
{
  constexpr auto const size = std::size_t{1337};

  auto a = sleip::dynamic_array<int>(size, 1337);

  auto const expected = sleip::dynamic_array<int>(size, -1);

  a.fill(-1);

  BOOST_TEST_ALL_EQ(a.begin(), a.end(), expected.begin(), expected.end());
}

void
test_swap()
{
  constexpr auto const size = std::size_t{16};

  auto a = sleip::dynamic_array<int>(size, 2);
  auto b = sleip::dynamic_array<int>(3 * size, -3);

  auto const* const old_a = a.data();
  auto const* const old_b = b.data();

  swap(a, b);

  BOOST_TEST_EQ(a.data(), old_b);
  BOOST_TEST_EQ(b.data(), old_a);

  BOOST_TEST_EQ(a.size(), 3 * size);
  BOOST_TEST_EQ(b.size(), size);

  swap(a, b);

  BOOST_TEST_EQ(a.data(), old_a);
  BOOST_TEST_EQ(b.data(), old_b);

  BOOST_TEST_EQ(a.size(), size);
  BOOST_TEST_EQ(b.size(), 3 * size);
}

int
main()
{
  test_fill();
  test_swap();

  return boost::report_errors();
}
