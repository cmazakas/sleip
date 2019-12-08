#include <sleip/dynamic_array.hpp>
#include <boost/core/lightweight_test.hpp>

void
test_reverse_range()
{
  {
    auto a         = sleip::dynamic_array<int>{1, 2, 3, 4, 5};
    auto reverse_a = sleip::dynamic_array<int>{5, 4, 3, 2, 1};

    BOOST_TEST_ALL_EQ(a.rbegin(), a.rend(), reverse_a.begin(), reverse_a.end());
  }

  {
    auto const a         = sleip::dynamic_array<int>{1, 2, 3, 4, 5};
    auto const reverse_a = sleip::dynamic_array<int>{5, 4, 3, 2, 1};

    BOOST_TEST_ALL_EQ(a.rbegin(), a.rend(), reverse_a.begin(), reverse_a.end());
  }

  {
    auto a         = sleip::dynamic_array<int>{1, 2, 3, 4, 5};
    auto reverse_a = sleip::dynamic_array<int>{5, 4, 3, 2, 1};

    BOOST_TEST_ALL_EQ(a.crbegin(), a.crend(), reverse_a.cbegin(), reverse_a.cend());
  }
}

int
main()
{
  test_reverse_range();
  return boost::report_errors();
}
