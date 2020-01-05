#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

void
test_equality()
{
  auto const a = sleip::dynamic_array<int[5]>(2, {1, 2, 3, 4, 5});
  auto const b = a;
  auto const c = sleip::dynamic_array<int[5]>(2, {1, 2, 3, 2, 1});

  BOOST_ASSERT((a == b));
  BOOST_ASSERT(!(a == c));

  BOOST_ASSERT(!(a != b));
  BOOST_ASSERT((a != c));
}

void
test_greater_than_less_than()
{
  auto const a = sleip::dynamic_array<int[5]>(2, {1, 2, 3, 4, 5});
  auto const b = a;
  auto const c = sleip::dynamic_array<int[5]>(2, {0, 1, 2, 1, 0});

  // test that c is less than a and that a is not less than c
  //
  BOOST_ASSERT((c < a));
  BOOST_ASSERT(!(a < c));

  // test that c is not greater than a and that a is greater than c
  //
  BOOST_ASSERT(!(c > a));
  BOOST_ASSERT((a > c));

  // test that a is not greater or less than b
  //
  BOOST_ASSERT(!(a > b));
  BOOST_ASSERT(!(a < b));

  // test that a is less than/greater than or equal to b
  //
  BOOST_ASSERT((a <= b));
  BOOST_ASSERT((a >= b));

  // test that c is less than or equal to a and that a is not less than or equal to c
  //
  BOOST_ASSERT((c <= a));
  BOOST_ASSERT(!(a <= c));

  // test that c is not greater than or equal t o a and that a is greater than or equal to c
  //
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
