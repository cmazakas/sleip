#include <sleip/dynamic_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <algorithm>

void
test_fill()
{
  auto a = sleip::dynamic_array<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto const expected = std::initializer_list<int>{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  a.fill(-1);

  BOOST_TEST(std::all_of(a.cbegin(), a.cend(), [](auto x) { return x == -1; }));
  BOOST_TEST_ALL_EQ(a.begin(), a.end(), expected.begin(), expected.end());
}

int
main()
{
  test_fill();

  return boost::report_errors();
}
