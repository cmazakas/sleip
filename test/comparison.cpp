#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

void
test_equality()
{
  auto const a = sleip::dynamic_array<int>{1, 2, 3, 4, 5};
  auto const b = a;
  auto const c = sleip::dynamic_array<int>{1, 2, 3, 2, 1};

  BOOST_ASSERT((a == b));
  BOOST_ASSERT(!(a == c));

  BOOST_ASSERT(!(a != b));
  BOOST_ASSERT((a != c));
}

void
test_greater_than_less_than()
{
  auto const a = sleip::dynamic_array<int>{1, 2, 3, 4, 5};
  auto const b = a;
  auto const c = sleip::dynamic_array<int>{0, 1, 2, 1, 0};

  BOOST_ASSERT((c < a));
  BOOST_ASSERT(!(a < c));

  BOOST_ASSERT(!(c > a));
  BOOST_ASSERT((a > c));

  BOOST_ASSERT(!(a > b));
  BOOST_ASSERT(!(a < b));

  BOOST_ASSERT((a <= b));
  BOOST_ASSERT((a >= b));

  BOOST_ASSERT((c <= a));
  BOOST_ASSERT(!(a <= c));

  BOOST_ASSERT(!(c >= a));
  BOOST_ASSERT((a >= c));
}

int
main()
{
  test_equality();
  test_greater_than_less_than();

  return boost::report_errors();
}
