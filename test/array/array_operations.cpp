#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>
#include <utility>
#include <type_traits>

using std::swap;
using std::begin;
using std::end;

void
test_fill()
{
  constexpr auto const size = 1337;

  int value[3] = {-1, -2, -3};

  auto a = sleip::dynamic_array<int[3]>(size, {1, 2, 3});
  a.fill(value);

  for (auto const& arr : a) { BOOST_TEST_ALL_EQ(begin(arr), end(arr), begin(value), end(value)); }
}

void
test_swap()
{
  auto const size = 16;

  auto a = sleip::dynamic_array<int[3]>(size, {1, 2, 3});
  auto b = sleip::dynamic_array<int[3]>(3 * size, {4, 5, 6});

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

void
test_member_swap()
{
  auto const size = 16;

  auto a = sleip::dynamic_array<int[3]>(size);
  auto b = sleip::dynamic_array<int[3]>(3 * size);

  auto const* const old_a = a.data();
  auto const* const old_b = b.data();

  a.swap(b);

  BOOST_TEST_EQ(a.data(), old_b);
  BOOST_TEST_EQ(b.data(), old_a);

  BOOST_TEST_EQ(a.size(), 3 * size);
  BOOST_TEST_EQ(b.size(), size);

  b.swap(a);

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
  test_member_swap();

  return boost::report_errors();
}
