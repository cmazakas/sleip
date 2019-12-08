#ifndef SLEIP_DYNAMIC_ARRAY_HPP_
#define SLEIP_DYNAMIC_ARRAY_HPP_

#include <boost/core/alloc_construct.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/throw_exception.hpp>
#include <boost/config.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <type_traits>

namespace sleip
{
namespace detail
{
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
} // namespace detail

template <class T, class Allocator = std::allocator<T>>
struct dynamic_array;

template <class T, class Allocator>
struct dynamic_array : boost::empty_value<Allocator, 0>
{
public:
  using value_type             = T;
  using allocator_type         = Allocator;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  using reference              = value_type&;
  using const_reference        = value_type const&;
  using pointer                = value_type*;
  using const_pointer          = value_type const*;
  using iterator               = pointer;
  using const_iterator         = const_pointer;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static_assert(std::is_object_v<value_type>, "Only support object types");
  static_assert(std::is_same_v<typename allocator_type::value_type, value_type>,
                "Allocator's value type must match container's");

private:
  struct dealloc
  {
    Allocator   alloc;
    std::size_t size = 0;

    dealloc(dealloc const&) = delete;

    dealloc(dealloc&& other) noexcept
      : alloc(std::move(other.get_allocator()))
      , size{other.size}
    {
      other.size = 0;
    };

    dealloc(Allocator const& alloc_, std::size_t size_)
      : alloc(alloc_)
      , size{size_}
    {
    }

    auto
    operator=(dealloc const&) & -> dealloc& = delete;

    auto
      operator=(dealloc&& other) &
      noexcept -> dealloc&
    {
      alloc = std::move(other.alloc);

      size       = other.size;
      other.size = 0;

      return *this;
    }

    auto
    operator()(T* ptr) -> void
    {
      std::allocator_traits<Allocator>::deallocate(alloc, ptr, size);
    }

    auto
    get_allocator() noexcept -> Allocator&
    {
      return alloc;
    }
  };

  T*          data_ = nullptr;
  std::size_t size_ = 0;

public:
  dynamic_array() noexcept(noexcept(Allocator())){};

  explicit dynamic_array(const Allocator& alloc) noexcept
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
  }

  dynamic_array(size_type count, T const& value, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    boost::alloc_construct_n(alloc_, d.get(), count, std::addressof(value), 1);

    data_ = d.release();
    size_ = count;
  }

  explicit dynamic_array(size_type count, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    boost::alloc_construct_n(alloc_, d.get(), count);

    data_ = d.release();
    size_ = count;
  }

  // impose Forward over Input because we can't resize the allocation so we need to know the
  // range's size up-front
  template <class ForwardIterator>
  dynamic_array(ForwardIterator first, ForwardIterator last, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    BOOST_CONCEPT_ASSERT((boost_concepts::ReadableIteratorConcept<ForwardIterator>) );
    BOOST_CONCEPT_ASSERT((boost_concepts::ForwardTraversalConcept<ForwardIterator>) );

    auto const count = static_cast<size_type>(std::distance(first, last));

    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

    boost::alloc_construct_n(alloc_, d.get(), count, first);

    data_ = d.release();
    size_ = count;
  }

  dynamic_array(dynamic_array const& other)
    : boost::empty_value<Allocator, 0>(
        boost::empty_init_t{},
        std::allocator_traits<allocator_type>::select_on_container_copy_construction(
          other.get_allocator()))
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
      dealloc(alloc_, other.size()));

    boost::alloc_construct_n(alloc_, d.get(), other.size(), other.begin());

    data_ = d.release();
    size_ = other.size();
  }

  dynamic_array(dynamic_array const& other, Allocator const& alloc)
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    auto d = std::unique_ptr<T[], dealloc>(
      std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
      dealloc(alloc_, other.size()));

    boost::alloc_construct_n(alloc_, d.get(), other.size(), other.begin());

    data_ = d.release();
    size_ = other.size();
  }

  dynamic_array(dynamic_array&& other) noexcept
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, std::move(other.get_allocator()))
    , data_(other.data_)
    , size_{other.size_}
  {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  dynamic_array(dynamic_array&& other, Allocator const& alloc)
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

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

    detail::alloc_move_construct_n(alloc_, d.get(), other.size(), other.begin());

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

    auto& alloc = boost::empty_value<Allocator, 0>::get();
    boost::alloc_destroy_n(alloc, data_, size_);
    std::allocator_traits<Allocator>::deallocate(alloc, data_, size_);
  }

  auto
  operator=(dynamic_array const& other) & -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    if (alloc_ == other.get_allocator()) {
      auto tmp = dynamic_array(other, alloc_);

      if constexpr (std::allocator_traits<
                      allocator_type>::propagate_on_container_copy_assignment::value) {
        alloc_ = other.get_allocator();
      }

      boost::alloc_destroy_n(alloc_, data_, size_);
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

    boost::alloc_destroy_n(alloc_, data_, size_);
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
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();

    if (alloc_ == other.get_allocator()) {
      if constexpr (std::allocator_traits<
                      Allocator>::propagate_on_container_move_assignment::value) {
        alloc_ = std::move(static_cast<boost::empty_value<Allocator, 0>&>(other).get());
      }

      boost::alloc_destroy_n(alloc_, data_, size_);
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

    detail::alloc_move_construct_n(a, d.get(), other.size(), other.begin());

    boost::alloc_destroy_n(alloc_, data_, size_);
    std::allocator_traits<Allocator>::deallocate(alloc_, data_, size_);

    data_  = d.release();
    size_  = count;
    alloc_ = a;

    return *this;
  }

  auto
  operator=(std::initializer_list<T> ilist) & -> dynamic_array&
  {
    auto& alloc_ = boost::empty_value<Allocator, 0>::get();
    auto  tmp    = dynamic_array(ilist, alloc_);

    boost::alloc_destroy_n(alloc_, data_, size_);
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
    return boost::empty_value<Allocator, 0>::get();
  }

  auto
  size() const noexcept -> size_type
  {
    return size_;
  }

  auto
  data() noexcept -> pointer
  {
    return data_;
  }

  auto
  data() const noexcept -> const_pointer
  {
    return data_;
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
    return *begin();
  }

  auto
  front() const& -> const_reference
  {
    return *cbegin();
  }

  auto
  back() & -> reference
  {
    auto tmp = end();
    --tmp;
    return *tmp;
  }

  auto
  back() const& -> const_reference
  {
    auto tmp = cend();
    --tmp;
    return *tmp;
  }
};
} // namespace sleip

#endif // SLEIP_DYNAMIC_ARRAY_HPP_
