#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <vector>

void
test_fill()
{
  constexpr auto const size = std::size_t{1337};

  auto a = sleip::dynamic_array<int>(size, 1337);

  auto const expected = sleip::dynamic_array<int>(size, -1);

  a.fill(-1);

  BOOST_TEST_ALL_EQ(a.begin(), a.end(), expected.begin(), expected.end());
}

int
main()
{
  test_fill();

  return boost::report_errors();
}
