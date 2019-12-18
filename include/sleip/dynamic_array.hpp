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

#include <boost/iterator/iterator_concepts.hpp>

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
// copy-paste-modify the great glenfe's .Core code
//
template <class A, class T, class I>
inline void
alloc_move_construct_n(A& a, T* p, std::size_t n, I b)
{
  ::boost::detail::alloc_destroyer<A, T> hold(a, p);
  for (std::size_t& i = hold.size(); i < n; void(++i), void(++b)) {
    std::allocator_traits<A>::construct(a, p + i, std::move_if_noexcept(*b));
  }
  hold.size() = 0;
}

// copy-paste-modify the iterator checking from libcxx's vector implementation
// see:
// https://github.com/llvm-mirror/libcxx/blob/8279a1399ec1db64f107b5f1d3966c3b8df28dd9/include/iterator
//
// for proper attribution to the authors and their work
//
template <class _Tp>
struct __has_iterator_category
{
private:
  struct __two
  {
    char __lx;
    char __lxx;
  };

  template <class _Up>
  static __two
  __test(...);

  template <class _Up>
  static char
  __test(typename _Up::iterator_category* = 0);

public:
  static const bool value = sizeof(__test<_Tp>(0)) == 1;
};

template <class _Tp, class _Up, bool = __has_iterator_category<std::iterator_traits<_Tp>>::value>
struct __has_iterator_category_convertible_to
  : public std::integral_constant<
      bool,
      std::is_convertible<typename std::iterator_traits<_Tp>::iterator_category, _Up>::value>

{
};

template <class _Tp, class _Up>
struct __has_iterator_category_convertible_to<_Tp, _Up, false> : public std::false_type
{
};

template <class _Tp>
struct __is_forward_iterator
  : public __has_iterator_category_convertible_to<_Tp, std::forward_iterator_tag>
{
};
} // namespace detail

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

  static_assert(std::is_object_v<value_type>, "Only support object types");
  static_assert(std::is_same_v<typename allocator_type::value_type, value_type>,
                "Allocator's value type must match container's");

private:
  struct dealloc
  {
    using pointer = typename dynamic_array::pointer;

    Allocator&  alloc;
    std::size_t size = 0;

    dealloc(Allocator& alloc_, std::size_t size_)
      : alloc(alloc_)
      , size{size_}
    {
    }

    auto
    operator()(pointer ptr) const -> void
    {
      std::allocator_traits<Allocator>::deallocate(alloc, ptr, size);
    }
  };

  pointer     data_ = nullptr;
  std::size_t size_ = 0;

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

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    auto* const p = boost::to_address(d.get());

    boost::alloc_construct_n(alloc_, p, count, std::addressof(value), 1);

    data_ = d.release();
    size_ = count;
  }

  explicit dynamic_array(size_type count, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    auto* const p = boost::to_address(d.get());

    boost::alloc_construct_n(alloc_, p, count);

    data_ = d.release();
    size_ = count;
  }

  // impose Forward over Input because we can't resize the allocation so we need to know the
  // range's size up-front
  template <class ForwardIterator,
            std::enable_if_t<detail::__is_forward_iterator<ForwardIterator>::value, int> = 0>
  dynamic_array(ForwardIterator first, ForwardIterator last, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    BOOST_CONCEPT_ASSERT((boost_concepts::ReadableIteratorConcept<ForwardIterator>) );
    BOOST_CONCEPT_ASSERT((boost_concepts::ForwardTraversalConcept<ForwardIterator>) );

    auto const count = static_cast<size_type>(std::distance(first, last));

    auto& alloc_ = boost::empty_value<Allocator>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    auto* const p = boost::to_address(d.get());

    boost::alloc_construct_n(alloc_, p, count, first);

    data_ = d.release();
    size_ = count;
  }

  dynamic_array(dynamic_array const& other)
    : boost::empty_value<Allocator>(
        boost::empty_init_t{},
        std::allocator_traits<allocator_type>::select_on_container_copy_construction(
          other.get_allocator()))
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
      dealloc(alloc_, other.size()));

    auto* const p = boost::to_address(d.get());

    boost::alloc_construct_n(alloc_, p, other.size(), other.begin());

    data_ = d.release();
    size_ = other.size();
  }

  dynamic_array(dynamic_array const& other, Allocator const& alloc)
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
      dealloc(alloc_, other.size()));

    auto* const p = boost::to_address(d.get());

    boost::alloc_construct_n(alloc_, p, other.size(), other.begin());

    data_ = d.release();
    size_ = other.size();
  }

  dynamic_array(dynamic_array&& other) noexcept
    : boost::empty_value<Allocator>(boost::empty_init_t{}, std::move(other.get_allocator()))
    , data_(other.data_)
    , size_{other.size_}
  {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  dynamic_array(dynamic_array&& other, Allocator const& alloc)
    : boost::empty_value<Allocator>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();

    if (alloc_ == other.get_allocator()) {
      data_       = other.data_;
      size_       = other.size_;
      other.data_ = nullptr;
      other.size_ = 0;
      return;
    }

    auto const count = other.size();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    auto* const p = boost::to_address(d.get());

    detail::alloc_move_construct_n(alloc_, p, other.size(), other.begin());

    data_ = d.release();
    size_ = count;
  }

  dynamic_array(std::initializer_list<T> init, Allocator const& alloc = Allocator())
    : dynamic_array(init.begin(), init.end(), alloc)
  {
  }

  ~dynamic_array()
  {
    if (data_ == nullptr && size_ == 0) { return; }

    auto& alloc = boost::empty_value<Allocator>::get();

    auto* const p = boost::to_address(data_);

    boost::alloc_destroy_n(alloc, p, size_);
    std::allocator_traits<Allocator>::deallocate(alloc, data_, size_);
  }

  auto
  operator=(dynamic_array const& other) & -> dynamic_array&
  {
    auto&       alloc_ = boost::empty_value<Allocator>::get();
    auto* const p      = boost::to_address(data_);

    if (alloc_ == other.get_allocator()) {
      auto tmp = dynamic_array(other, alloc_);

      if constexpr (std::allocator_traits<
                      allocator_type>::propagate_on_container_copy_assignment::value) {
        alloc_ = other.get_allocator();
      }

      boost::alloc_destroy_n(alloc_, p, size_);
      std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

      data_ = tmp.data_;
      size_ = tmp.size_;

      tmp.data_ = nullptr;
      tmp.size_ = 0;
      return *this;
    }

    auto a = std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value
               ? other.get_allocator()
               : alloc_;

    auto tmp = dynamic_array(other.begin(), other.end(), a);

    boost::alloc_destroy_n(alloc_, p, size_);
    std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

    data_ = tmp.data_;
    size_ = tmp.size_;

    tmp.data_ = nullptr;
    tmp.size_ = 0;

    if constexpr (std::allocator_traits<
                    allocator_type>::propagate_on_container_copy_assignment::value) {
      alloc_ = other.get_allocator();
    }

    return *this;
  }

  auto
    operator=(dynamic_array&& other) &
    noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
             std::allocator_traits<Allocator>::is_always_equal::value) -> dynamic_array&
  {
    auto&       alloc_ = boost::empty_value<Allocator>::get();
    auto* const p      = boost::to_address(data_);

    if (alloc_ == other.get_allocator()) {
      if constexpr (std::allocator_traits<
                      Allocator>::propagate_on_container_move_assignment::value) {
        alloc_ = std::move(static_cast<boost::empty_value<Allocator>&>(other).get());
      }

      boost::alloc_destroy_n(alloc_, p, size_);
      std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

      data_ = other.data_;
      size_ = other.size_;

      other.data_ = nullptr;
      other.size_ = 0;

      return *this;
    }

    auto a = std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value
               ? other.get_allocator()
               : alloc_;

    auto const count = other.size();

    auto d = std::unique_ptr<T[], dealloc>(std::allocator_traits<Allocator>::allocate(a, count),
                                           dealloc(a, count));

    detail::alloc_move_construct_n(a, boost::to_address(d.get()), other.size(), other.begin());

    boost::alloc_destroy_n(alloc_, p, size_);
    std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

    data_  = d.release();
    size_  = count;
    alloc_ = a;

    return *this;
  }

  auto
  operator=(std::initializer_list<T> ilist) & -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator>::get();
    auto  tmp    = dynamic_array(ilist, alloc_);

    auto* const p = boost::to_address(data_);

    boost::alloc_destroy_n(alloc_, p, size_);
    std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

    data_ = tmp.data_;
    size_ = tmp.size_;

    tmp.data_ = nullptr;
    tmp.size_ = 0;

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

  auto operator[](size_type pos) & -> reference { return data_[pos]; }
  auto operator[](size_type pos) const& -> const_reference { return data_[pos]; }

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
    constexpr auto const unroll_size = 8;
    auto const           num_rolls   = size_ / unroll_size;

    for (size_type idx = 0; idx < num_rolls; ++idx) {
      data_[idx * unroll_size + 0] = value;
      data_[idx * unroll_size + 1] = value;
      data_[idx * unroll_size + 2] = value;
      data_[idx * unroll_size + 3] = value;
      data_[idx * unroll_size + 4] = value;
      data_[idx * unroll_size + 5] = value;
      data_[idx * unroll_size + 6] = value;
      data_[idx * unroll_size + 7] = value;
    }

    for (size_type idx = num_rolls * unroll_size; idx < size_; ++idx) { data_[idx] = value; }
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
    }

    auto tmp_data = data_;
    auto tmp_size = size_;

    data_ = other.data_;
    size_ = other.size_;

    other.data_ = tmp_data;
    other.size_ = tmp_size;
  }
};

template <class T, class Allocator>
auto
operator==(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  if (lhs.size() != rhs.size()) { return false; }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, class Allocator>
auto
operator>(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool
{
  return !(lhs < rhs);
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
