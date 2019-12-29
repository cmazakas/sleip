#include <sleip/dynamic_array.hpp>

#include <boost/core/default_allocator.hpp>

#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>

#include <memory>
#include <iterator>
#include <algorithm>
#include <vector>
#include <string>
#include <array>
#include <string_view>

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

namespace pmr = boost::container::pmr;

void
test_default_constructible()
{
  sleip::dynamic_array<int[16]> buf;

  BOOST_TEST_EQ(buf.size(), 0);
  BOOST_TEST_EQ(std::distance(buf.begin(), buf.end()), 0);
  BOOST_TEST_EQ(buf.data(), nullptr);
  BOOST_TEST(buf.get_allocator() == std::allocator<int>{});

  sleip::dynamic_array<int[16], pmr::polymorphic_allocator<int[16]>> pmr_buf;

  BOOST_TEST_EQ(pmr_buf.size(), 0);
  BOOST_TEST_EQ(std::distance(pmr_buf.begin(), pmr_buf.end()), 0);
  BOOST_TEST_EQ(pmr_buf.data(), nullptr);
  BOOST_TEST(pmr_buf.get_allocator() == pmr::polymorphic_allocator<int>{});
}

void
test_allocator_constructor()
{
  auto alloc = boost::default_allocator<int[16]>();

  static_assert(std::is_same_v<typename decltype(alloc)::value_type,
                               typename sleip::dynamic_array<int[16]>::value_type>);

  sleip::dynamic_array<int[16], boost::default_allocator<int[16]>> buf(alloc);

  BOOST_TEST(buf.get_allocator() == alloc);

  BOOST_TEST_EQ(buf.size(), 0);
  BOOST_TEST_EQ(std::distance(buf.begin(), buf.end()), 0);
  BOOST_TEST_EQ(buf.data(), nullptr);
}

void
test_value_constructible()
{
  auto const count = 24;

  int value[2] = {-1, -1};

  sleip::dynamic_array<int[2]> buf(count, value);

  BOOST_TEST_EQ(buf.size(), count);
  BOOST_TEST(
    std::all_of(buf.begin(), buf.end(), [=](auto&& v) { return v[0] == -1 && v[1] == -1; }));
}

#ifdef BOOST_NO_EXCEPTIONS
void
test_value_constructible_throwing()
{
}
#else
void
test_value_constructible_throwing()
{
  struct throwing
  {
    std::size_t&         idx;
    std::array<char, 6>& c_out;
    std::array<char, 6>& d_out;

    throwing() = delete;
    throwing(throwing const& other)
      : idx{other.idx}
      , c_out(other.c_out)
      , d_out(other.d_out)
    {
      if ((idx + 1) == 6) { throw 42; }

      c_out[idx] = static_cast<char>('a' + idx);
      ++idx;
    }

    throwing(throwing&&) = delete;

    ~throwing()
    {
      if (idx == std::size_t(-1)) { return; }
      d_out[5 - idx] = static_cast<char>('a' + idx - 1);

      --idx;
    }

    throwing(std::size_t& idx_, std::array<char, 6>& c_out_, std::array<char, 6>& d_out_)
      : idx(idx_)
      , c_out(c_out_)
      , d_out(d_out_)
    {
    }
  };

  auto constructor_out = std::array<char, 6>{};
  auto destructor_out  = std::array<char, 6>{};
  auto idx             = std::size_t{0};

  auto const value = throwing(idx, constructor_out, destructor_out);

  auto const expected_c_out = std::string_view("abcde");
  auto const expected_d_out = std::string_view("edcba");

  BOOST_TEST_THROWS(
    (sleip::dynamic_array<throwing[3][2]>(1, {{value, value}, {value, value}, {value, value}})),
    int);

  BOOST_TEST_ALL_EQ(constructor_out.begin(), constructor_out.end() - 1, expected_c_out.begin(),
                    expected_c_out.end());

  BOOST_TEST_ALL_EQ(destructor_out.begin(), destructor_out.end() - 1, expected_d_out.begin(),
                    expected_d_out.end());
}
#endif

void
test_size_constructible()
{
  auto const count = 24;

  sleip::dynamic_array<int[16]> buf(count);

  BOOST_TEST_EQ(buf.size(), 24);
  BOOST_TEST(std::all_of(buf.begin(), buf.end(), [](auto const& v) {
    return std::all_of(std::begin(v), std::end(v), [](auto const w) { return w == int{}; });
  }));
}

#ifdef BOOST_NO_EXCEPTIONS
void
test_size_constructible_throwing()
{
}
#else
void
test_size_constructible_throwing()
{
  struct foo_throwing
  {
    foo_throwing() { throw 42; }
    foo_throwing(foo_throwing const&) = delete;
    foo_throwing(foo_throwing&&)      = delete;
  };

  auto const count = 24;

  BOOST_TEST_THROWS((sleip::dynamic_array<foo_throwing[4]>(count)), int);
}
#endif

void
test_range_constructible()
{
  auto const count = 16;

  auto const rows = 2;
  auto const cols = 5;

  int value[rows][cols] = {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}};

  sleip::dynamic_array<int[rows][cols]> x(count, value);
  BOOST_TEST_EQ(x.size(), count);

  sleip::dynamic_array<int[rows][cols]> y(x.begin(), x.end());
  BOOST_TEST_EQ(y.size(), x.size());

  auto pos = y.begin();
  for (auto i = 0; i < count; ++i) {
    auto const& arr = *pos;
    for (auto r = 0; r < rows; ++r) {
      for (auto c = 0; c < cols; ++c) { BOOST_TEST_EQ(arr[r][c], value[r][c]); }
    }
    ++pos;
  }

  BOOST_TEST_EQ((pos - y.begin()), y.size());
}

void
test_copy_constructible()
{
  int value[5]{1, 2, 3, 4, 5};

  sleip::dynamic_array<int[5]> const x(8, value);
  sleip::dynamic_array<int[5]> const y(x);

  BOOST_TEST_EQ(x.size(), 8);
  BOOST_TEST_EQ(y.size(), x.size());

  for (auto const& arr : y) {
    BOOST_TEST(std::equal(std::begin(arr), std::end(arr), std::begin(value), std::end(value)));
  }
}

void
test_copy_constructible_allocator()
{
  int value[5]{1, 2, 3, 4, 5};

  sleip::dynamic_array<int[5], boost::default_allocator<int[5]>> const x(
    1, value, boost::default_allocator<int[5]>());

  sleip::dynamic_array<int[5], boost::default_allocator<int[5]>> const y(
    x, boost::default_allocator<int[5]>());

  BOOST_TEST_EQ(y.size(), x.size());

  auto const& arr = *(y.begin());
  BOOST_TEST_ALL_EQ(std::begin(arr), std::end(arr), std::begin(value), std::end(value));
}

void
test_move_constructible()
{
  auto const count = 16;

  int value[2][2]{{1, 2}, {3, 4}};

  sleip::dynamic_array<int[2][2]> x(count, value);

  auto const* const old_data = x.data();

  sleip::dynamic_array<int[2][2]> const y(std::move(x));

  static_assert(noexcept(sleip::dynamic_array<int[2][2]>(std::move(x))));

  BOOST_TEST(x.empty());
  BOOST_TEST_EQ(y.size(), count);
  BOOST_TEST_EQ(y.data(), old_data);
}

void
test_move_constructible_allocator()
{
  auto pool       = pmr::unsynchronized_pool_resource();
  auto pool_alloc = pmr::polymorphic_allocator<std::unique_ptr<int>[5]>(&pool);

  auto alloc = pmr::polymorphic_allocator<std::unique_ptr<int>[5]>();

  auto const count = 16;

  sleip::dynamic_array<std::unique_ptr<int>[5], pmr::polymorphic_allocator<std::unique_ptr<int>[5]>>
    x(count, pool_alloc);

  for (auto& arr : x) {
    for (std::size_t i = 0; i < 5; ++i) { arr[i] = std::make_unique<int>(1337); }
  }

  sleip::dynamic_array<std::unique_ptr<int>[5], pmr::polymorphic_allocator<std::unique_ptr<int>[5]>>
    y(std::move(x), alloc);

  BOOST_TEST(x.get_allocator() != y.get_allocator());
  BOOST_TEST_EQ(x.size(), count);
  BOOST_TEST_EQ(y.size(), count);

  for (auto& arr : x) {
    for (std::size_t i = 0; i < 5; ++i) { BOOST_TEST(!arr[i]); }
  }

  for (auto& arr : y) {
    for (std::size_t i = 0; i < 5; ++i) { BOOST_TEST_EQ(*arr[i], 1337); }
  }
}

void
test_initializer_list_constructible()
{
  auto buf = sleip::dynamic_array<int[3]>{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

  int const expected[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

  BOOST_TEST_EQ(buf.size(), 3);

  BOOST_TEST(std::equal(buf.begin(), buf.end(), std::begin(expected), std::end(expected),
                        [](auto const& a, auto const& b) {
                          return std::equal(std::begin(a), std::end(a), std::begin(b));
                        }));
}

int
main()
{
  test_default_constructible();
  test_allocator_constructor();
  test_value_constructible();
  test_value_constructible_throwing();
  test_size_constructible();
  test_size_constructible_throwing();
  test_range_constructible();
  test_copy_constructible();
  test_copy_constructible_allocator();
  test_move_constructible();
  test_move_constructible_allocator();
  test_initializer_list_constructible();

  return boost::report_errors();
}
