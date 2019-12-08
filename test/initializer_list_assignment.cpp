#include <sleip/dynamic_array.hpp>

#include <vector>

#include <boost/core/lightweight_test.hpp>

void
test_initializer_list_assignment()
{
  // empty
  //
  {
    auto a    = sleip::dynamic_array<int>();
    auto list = std::initializer_list<int>{1, 2, 3};

    a = list;

    BOOST_TEST_EQ(a.size(), list.size());
    BOOST_TEST_ALL_EQ(a.begin(), a.end(), list.begin(), list.end());
  }

  // non-empty
  {
    auto a    = sleip::dynamic_array<std::vector<int>>(8);
    auto list = std::initializer_list<std::vector<int>>{std::vector<int>()};

    BOOST_TEST_EQ(a.size(), 8);

    a = list;

    BOOST_TEST_EQ(a.size(), 1);
  }
}

int
main()
{
  test_initializer_list_assignment();
  return boost::report_errors();
}
