#ifndef SLEIP_DYNAMIC_ARRAY_HPP_
#define SLEIP_DYNAMIC_ARRAY_HPP_

#include <sleip/dynamic_array_fwd.hpp>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>

#include <boost/core/alloc_construct.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/core/noinit_adaptor.hpp>
#include <boost/core/pointer_traits.hpp>
#include <boost/core/first_scalar.hpp>

#include <boost/type_traits/is_bounded_array.hpp>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#include <boost/mp11/function.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace sleip
{
namespace detail
{
template <class T>
inline constexpr std::size_t array_size_v = 1;

template <class T, std::size_t N>
inline constexpr std::size_t array_size_v<T[N]> = sizeof(T[N]) /
                                                  sizeof(std::remove_all_extents_t<T[N]>);

template <class T>
constexpr auto
num_elems(std::size_t const count) noexcept -> std::size_t
{
  return count * array_size_v<T>;
}

template <class Iterator>
struct move_if_noexcept_adaptor
{
  Iterator it;

  auto operator*() & -> decltype(auto) { return std::move_if_noexcept(*it); }

  auto
  operator++() & -> move_if_noexcept_adaptor&
  {
    ++it;
    return *this;
  }
};

template <class Iterator>
struct array_walker
{
  Iterator it;

  std::size_t step = 0;

  auto operator*() & -> decltype(auto)
  {
    auto* const arr = boost::first_scalar(std::addressof(*it));
    return arr[step];
  }

  auto
  operator++() & -> array_walker&
  {
    if ((step + 1) == array_size_v<typename std::iterator_traits<Iterator>::value_type>) {
      ++it;
      step = 0;
    } else {
      ++step;
    }

    return *this;
  }
};

template <class It, class To>
using is_category_convertible_ =
  std::is_convertible<typename std::iterator_traits<It>::iterator_category, To>;

template <class It, class To>
using is_category_convertible =
  boost::mp11::mp_eval_or<boost::mp11::mp_false, is_category_convertible_, It, To>;

template <class It>
using is_forward_iterator = is_category_convertible<It, std::forward_iterator_tag>;

template <class It>
inline constexpr bool const is_forward_iterator_v = is_forward_iterator<It>::value;

using std::begin;
using std::end;

template <class Range>
using has_begin = is_forward_iterator<decltype(begin(std::declval<Range const&>()))>;

template <class Range>
using has_end = is_forward_iterator<decltype(end(std::declval<Range const&>()))>;

template <class Range>
using has_begin_end = boost::mp11::mp_all<has_begin<Range>, has_end<Range>>;

template <class Range>
using is_range = boost::mp11::mp_eval_or<boost::mp11::mp_false, has_begin_end, Range>;

template <class Range>
inline constexpr bool const is_range_v = is_range<Range>::value;

} // namespace detail

struct noinit_t
{
};

inline constexpr noinit_t noinit;

template <class T, class Allocator>
struct dynamic_array : boost::empty_value<Allocator>
{
public:
  using value_type             = T;
  using allocator_type         = Allocator;
  using size_type              = typename std::allocator_traits<Allocator>::size_type;
  using difference_type        = typename std::allocator_traits<Allocator>::difference_type;
  using reference              = value_type&;
  using const_reference        = value_type const&;
  using pointer                = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer          = typename std::allocator_traits<Allocator>::const_pointer;
  using iterator               = value_type*;
  using const_iterator         = value_type const*;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static_assert(
    std::is_object_v<T> && (!std::is_array_v<T> || boost::is_bounded_array<T>::value),
    "Only support object types, including bound array types. Unbound arrays are not supported");

  static_assert(std::is_same_v<typename allocator_type::value_type, value_type>,
                "Allocator's value type must match container's");

private:
  pointer     data_ = nullptr;
  std::size_t size_ = 0;

  template <typename Allocator_, typename... Args>
  static pointer
  create_(Allocator_& alloc, std::size_t count, Args&&... args)
  {
    struct dealloc
    {
      using pointer = typename dynamic_array::pointer;

      Allocator&  alloc;
      std::size_t size = 0;
      auto
      operator()(pointer ptr) const -> void
      {
        std::allocator_traits<Allocator>::deallocate(alloc, ptr, size);
      }
    };

    auto d = std::unique_ptr<T[], dealloc>(std::allocator_traits<Allocator>::allocate(alloc, count),
                                           dealloc{alloc, count});

    auto* const p = boost::first_scalar(boost::to_address(d.get()));
    boost::alloc_construct_n(alloc, p, detail::num_elems<T>(count), std::forward<Args>(args)...);

    return d.release();
  }

  template <typename Allocator_>
  static void
  destroy_(Allocator_& alloc, pointer data, std::size_t count)
  {
    if (data == nullptr) { return; }

    auto* const p = boost::first_scalar(boost::to_address(data));
    boost::alloc_destroy_n(alloc, p, detail::num_elems<T>(count));

    std::allocator_traits<Allocator>::deallocate(alloc, data, count);
  }

public:
  dynamic_array() noexcept(noexcept(Allocator()))
    : boost::empty_value<Allocator>(boost::empty_init_t{}){};

  explicit dynamic_array(const Allocator& alloc) noexcept
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
  }

  dynamic_array(size_type count, T const& value, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    data_ =
      create_(alloc_, count, boost::first_scalar(std::addressof(value)), detail::num_elems<T>(1));
    size_ = count;
  }

  explicit dynamic_array(size_type count, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    data_        = create_(alloc_, count);
    size_        = count;
  }

  explicit dynamic_array(size_type count, noinit_t, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    auto  a      = boost::noinit_adapt(alloc_);
    data_        = create_(a, count);
    size_        = count;
  }

  // impose Forward over Input because we can't resize the allocation so we need to know the range's
  // size up-front
  //
  template <class ForwardIterator,
            std::enable_if_t<detail::is_forward_iterator_v<ForwardIterator>, int> = 0>
  dynamic_array(ForwardIterator first, ForwardIterator last, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto const count = static_cast<size_type>(std::distance(first, last));

    auto& alloc_ = boost::empty_value<Allocator>::get();
    data_        = create_(alloc_, count, detail::array_walker<ForwardIterator>{first});
    size_        = count;
  }

  dynamic_array(dynamic_array const& other)
    : boost::empty_value<Allocator>(
        boost::empty_init_t{},
        std::allocator_traits<allocator_type>::select_on_container_copy_construction(
          other.get_allocator()))
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    data_        = create_(alloc_, other.size(), boost::first_scalar(other.data()));
    size_        = other.size();
  }

  dynamic_array(dynamic_array const& other, Allocator const& alloc)
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    data_        = create_(alloc_, other.size(), boost::first_scalar(other.data()));
    size_        = other.size();
  }

  dynamic_array(dynamic_array&& other) noexcept
    : boost::empty_value<Allocator>(boost::empty_init_t{}, std::move(other.get_allocator()))
  {
    data_ = std::exchange(other.data_, nullptr);
    size_ = std::exchange(other.size_, 0);
  }

  dynamic_array(dynamic_array&& other, Allocator const& alloc)
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    if (alloc_ == other.get_allocator()) {
      data_ = std::exchange(other.data_, nullptr);
      size_ = std::exchange(other.size_, 0);
      return;
    }

    data_ = create_(alloc_, other.size(),
                    detail::move_if_noexcept_adaptor<detail::array_walker<iterator>>{
                      detail::array_walker<iterator>{other.begin()}});
    size_ = other.size();
  }

  dynamic_array(std::initializer_list<T> init, Allocator const& alloc = Allocator())
    : dynamic_array(init.begin(), init.end(), alloc)
  {
  }

  template <class Range, std::enable_if_t<detail::is_range_v<Range>, int> = 0>
  dynamic_array(Range const& range)
    : boost::empty_value<Allocator>(boost::empty_init_t{})
  {
    using std::begin;
    using std::end;

    *this = dynamic_array(begin(range), end(range));
  }

  ~dynamic_array()
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    destroy_(alloc_, data_, size_);
  }

  auto
  operator=(dynamic_array const& other) & -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    if (alloc_ == other.get_allocator()) {
      auto tmp = dynamic_array(other, alloc_);
      destroy_(alloc_, data_, size_);

      if constexpr (std::allocator_traits<
                      allocator_type>::propagate_on_container_copy_assignment::value) {
        alloc_ = other.get_allocator();
      }
      data_ = std::exchange(tmp.data_, nullptr);
      size_ = std::exchange(tmp.size_, 0);

      return *this;
    }

    auto a = std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value
               ? other.get_allocator()
               : alloc_;

    auto tmp = dynamic_array(other.begin(), other.end(), a);
    destroy_(alloc_, data_, size_);

    if constexpr (std::allocator_traits<
                    allocator_type>::propagate_on_container_copy_assignment::value) {
      alloc_ = other.get_allocator();
    }
    data_ = std::exchange(tmp.data_, nullptr);
    size_ = std::exchange(tmp.size_, 0);

    return *this;
  }

  auto
    operator=(dynamic_array&& other) &
    noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
             std::allocator_traits<Allocator>::is_always_equal::value) -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    if (alloc_ == other.get_allocator()) {
      destroy_(alloc_, data_, size_);

      if constexpr (std::allocator_traits<
                      Allocator>::propagate_on_container_move_assignment::value) {
        alloc_ = std::move(static_cast<boost::empty_value<Allocator>&>(other).get());
      }

      data_ = std::exchange(other.data_, nullptr);
      size_ = std::exchange(other.size_, 0);

      return *this;
    }

    destroy_(alloc_, data_, size_);

    auto a = std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value
               ? other.get_allocator()
               : alloc_;
    data_  = create_(a, other.size(),
                    detail::move_if_noexcept_adaptor<std::remove_all_extents_t<T>*>{
                      boost::first_scalar(other.data())});
    size_  = other.size();
    alloc_ = a;

    return *this;
  }

  auto
  operator=(std::initializer_list<T> ilist) & -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    auto  tmp    = dynamic_array(ilist, alloc_);

    destroy_(alloc_, data_, size_);

    data_ = std::exchange(tmp.data_, nullptr);
    size_ = std::exchange(tmp.size_, 0);

    return *this;
  }

  auto
  get_allocator() const -> allocator_type
  {
    return boost::empty_value<Allocator>::get();
  }

  auto
  size() const noexcept -> size_type
  {
    return size_;
  }

  auto
  data() noexcept -> T*
  {
    return boost::to_address(data_);
  }

  auto
  data() const noexcept -> T const*
  {
    return boost::to_address(data_);
  }

  auto
  begin() noexcept -> iterator
  {
    return iterator{data()};
  }

  auto
  begin() const noexcept -> const_iterator
  {
    return const_iterator{data()};
  }

  auto
  cbegin() const noexcept -> const_iterator
  {
    return const_iterator{data()};
  }

  auto
  end() noexcept -> iterator
  {
    return iterator{data() + size()};
  }

  auto
  end() const noexcept -> const_iterator
  {
    return const_iterator{data() + size()};
  }

  auto
  cend() const noexcept -> const_iterator
  {
    return const_iterator{data() + size()};
  }

  auto
  rbegin() noexcept -> reverse_iterator
  {
    return std::make_reverse_iterator(end());
  }

  auto
  rbegin() const noexcept -> const_reverse_iterator
  {
    return std::make_reverse_iterator(cend());
  }

  auto
  crbegin() const noexcept -> const_reverse_iterator
  {
    return std::make_reverse_iterator(cend());
  }

  auto
  rend() noexcept -> reverse_iterator
  {
    return std::make_reverse_iterator(begin());
  }

  auto
  rend() const noexcept -> const_reverse_iterator
  {
    return std::make_reverse_iterator(cbegin());
  }

  auto
  crend() const noexcept -> const_reverse_iterator
  {
    return std::make_reverse_iterator(cbegin());
  }

  auto
  at(size_type pos) & -> reference
  {
    if (!(pos < size())) {
      boost::throw_exception(
        std::out_of_range("sleip::dynamic_array::at -> size_type pos is larger than size()"));
    }

    return data_[pos];
  }

  auto
  at(size_type pos) const& -> const_reference
  {
    if (!(pos < size())) {
      boost::throw_exception(
        std::out_of_range("sleip::dynamic_array::at -> size_type pos is larger than size()"));
    }

    return data_[pos];
  }

  auto operator[](size_type pos) & -> reference
  {
    BOOST_ASSERT(pos < size());
    return data_[pos];
  }

  auto operator[](size_type pos) const& -> const_reference
  {
    BOOST_ASSERT(pos < size());
    return data_[pos];
  }

  auto
  front() & -> reference
  {
    BOOST_ASSERT(!empty());
    return *begin();
  }

  auto
  front() const& -> const_reference
  {
    BOOST_ASSERT(!empty());
    return *cbegin();
  }

  auto
  back() & -> reference
  {
    BOOST_ASSERT(!empty());
    auto tmp = end();
    --tmp;
    return *tmp;
  }

  auto
  back() const& -> const_reference
  {
    BOOST_ASSERT(!empty());
    auto tmp = cend();
    --tmp;
    return *tmp;
  }

  auto
  empty() const noexcept -> bool
  {
    return size_ == 0;
  }

  auto
  max_size() const noexcept -> size_type
  {
    return -1;
  }

  auto
  fill(T const& value) -> void
  {
    if constexpr (std::is_array_v<T>) {
      std::for_each(begin(), end(), [&](auto& arr) {
        std::copy(std::begin(value), std::end(value), std::begin(arr));
      });
    } else {
      std::fill(begin(), end(), value);
    }
  }

  auto
    swap(dynamic_array& other) &
    noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
             std::allocator_traits<Allocator>::is_always_equal::value) -> void
  {
    if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
      auto& alloc_       = boost::empty_value<Allocator>::get();
      auto& other_alloc_ = static_cast<boost::empty_value<Allocator>&>(other).get();
      swap(alloc_, other_alloc_);
    } else {
      BOOST_ASSERT(get_allocator() == other.get_allocator());
    }

    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }
};

template <class T, class Allocator>
auto
operator==(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  auto a = boost::first_scalar(lhs.data());
  auto b = boost::first_scalar(rhs.data());

  return std::equal(a, a + detail::num_elems<T>(lhs.size()), b,
                    b + detail::num_elems<T>(rhs.size()));
}

template <class T, class Allocator>
auto
operator!=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  return !(lhs == rhs);
}

template <class T, class Allocator>
auto
operator<(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  if (lhs.size() != rhs.size()) { return false; }
  auto a = boost::first_scalar(lhs.data());
  auto b = boost::first_scalar(rhs.data());

  return std::lexicographical_compare(a, a + detail::num_elems<T>(lhs.size()), b,
                                      b + detail::num_elems<T>(rhs.size()));
}

template <class T, class Allocator>
auto
operator>(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  return rhs < lhs;
}

template <class T, class Allocator>
auto
operator<=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  return !(rhs < lhs);
}

template <class T, class Allocator>
auto
operator>=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  return !(lhs < rhs);
}
} // namespace sleip

#endif // SLEIP_DYNAMIC_ARRAY_HPP_
