#include <sleip/dynamic_array.hpp>
#include <boost/core/lightweight_test.hpp>

#ifdef BOOST_NO_EXCEPTIONS

#include <iostream>
#include <exception>

namespace boost
{
void
throw_exception(std::exception const& e)
{
  std::cerr << "Exception generated in noexcept code\nError: " << e.what() << "\n\n";
  std::terminate();
}
} // namespace boost
#endif

void
test_at()
{
  // in range, non-const
  //
  {
    auto  a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto& x = a.at(0)[0];
    BOOST_TEST_EQ(x, 1);

    x = 1337;

    BOOST_TEST_EQ((*a.begin())[0], 1337);
  }

  // in range, const
  //
  {
    auto const  a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto const& x = a.at(0)[1];
    BOOST_TEST_EQ(x, 2);
  }

  // out of range
  //
  {
    auto a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    BOOST_TEST_THROWS((a.at(1337)), std::out_of_range);
  }
}

void
test_subscript()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int[3]>(16, {1, 2, 3});
    auto& x = a[13][0];
    BOOST_TEST_EQ(x, 1);

    x = 1337;

    BOOST_TEST_EQ((*(a.begin() + 13))[0], 1337);
  }

  // const
  //
  {
    auto const  a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto const& x = a[0][2];
    BOOST_TEST_EQ(x, 3);
  }

  // const multidimensional
  {
    auto const  a = sleip::dynamic_array<int[3][3][3]>(1);
    auto const& x = a[0][0][1][2];
    BOOST_TEST_EQ(x, int{});
  }
}

void
test_front()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto& x = a.front();
    BOOST_TEST_EQ(x[0], 1);
  }

  // const
  //
  {
    auto        a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto const& x = a.front();
    BOOST_TEST_EQ(x[0], 1);
  }
}

void
test_back()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto& x = a.back();
    BOOST_TEST_EQ(x[2], 3);
  }

  // const
  //
  {
    auto        a = sleip::dynamic_array<int[3]>(1, {1, 2, 3});
    auto const& x = a.back();
    BOOST_TEST_EQ(x[2], 3);
  }
}

int
main()
{
  test_at();
  test_subscript();
  test_front();
  test_back();

  return boost::report_errors();
}
