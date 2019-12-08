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
    auto  a = sleip::dynamic_array<int>{1, 2, 3};
    auto& x = a.at(0);
    BOOST_TEST_EQ(x, 1);
  }

  // in range, const
  //
  {
    auto const  a = sleip::dynamic_array<int>{1, 2, 3};
    auto const& x = a.at(1);
    BOOST_TEST_EQ(x, 2);
  }

  // out of range
  //
  {
    auto a = sleip::dynamic_array<int>{1, 2, 3};
    BOOST_TEST_THROWS((a.at(1337)), std::out_of_range);
  }
}

void
test_subscript()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int>{1, 2, 3};
    auto& x = a[0];
    BOOST_TEST_EQ(x, 1);
  }

  // const
  //
  {
    auto const  a = sleip::dynamic_array<int>{1, 2, 3};
    auto const& x = a[2];
    BOOST_TEST_EQ(x, 3);
  }
}

void
test_front()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int>{1, 2, 3};
    auto& x = a.front();
    BOOST_TEST_EQ(x, 1);
  }

  // const
  //
  {
    auto const  a = sleip::dynamic_array<int>{1, 2, 3};
    auto const& x = a.front();
    BOOST_TEST_EQ(x, 1);
  }
}

void
test_back()
{
  // non-const
  //
  {
    auto  a = sleip::dynamic_array<int>{1, 2, 3};
    auto& x = a.back();
    BOOST_TEST_EQ(x, 3);
  }

  // const
  //
  {
    auto const  a = sleip::dynamic_array<int>{1, 2, 3};
    auto const& x = a.back();
    BOOST_TEST_EQ(x, 3);
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
