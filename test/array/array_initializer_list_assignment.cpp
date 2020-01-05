#include <sleip/dynamic_array.hpp>

#include <vector>
#include <algorithm>

#include <boost/core/lightweight_test.hpp>

using std::begin;
using std::end;

void
test_initializer_list_assignment()
{
#ifndef BOOST_GCC
  // empty
  //
  {
    int value[3] = {1, 2, 3};

    auto a    = sleip::dynamic_array<int[3]>();
    auto list = std::initializer_list<int[3]>{{1, 2, 3}};

    a = list;

    BOOST_TEST_EQ(a.size(), list.size());
    for (auto const& arr : a) { BOOST_TEST_ALL_EQ(begin(arr), end(arr), begin(value), end(value)); }
  }

  // non-empty
  {
    auto a    = sleip::dynamic_array<int[3]>(8);
    auto list = std::initializer_list<int[3]>{{1, 2, 3}};

    BOOST_TEST_EQ(list.size(), 1);
    BOOST_TEST_EQ(a.size(), 8);

    a = list;

    BOOST_TEST_EQ(a.size(), 1);
  }
#endif
}

int
main()
{
  test_initializer_list_assignment();
  return boost::report_errors();
}
