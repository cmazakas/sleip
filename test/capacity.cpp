#include <sleip/dynamic_array.hpp>

#include <boost/core/lightweight_test.hpp>

void
test_empty()
{
  auto a = sleip::dynamic_array<int>();

  BOOST_TEST(a.empty());

  a = sleip::dynamic_array<int>{1, 2, 3};
  BOOST_TEST(!a.empty());
}

void
test_size()
{
  auto a = sleip::dynamic_array<int>();
  BOOST_TEST_EQ(a.size(), 0);

  a = sleip::dynamic_array<int>{1, 2, 3};
  BOOST_TEST_EQ(a.size(), 3);
}

void
test_max_size()
{
  auto a = sleip::dynamic_array<int>();
  BOOST_TEST_EQ(a.max_size(), std::size_t(-1));
}

int
main()
{
  test_empty();
  test_size();
  test_max_size();

  return boost::report_errors();
}
