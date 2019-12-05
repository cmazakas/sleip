#ifndef SLEIP_DYNAMIC_ARRAY_HPP_
#define SLEIP_DYNAMIC_ARRAY_HPP_

#include <boost/core/alloc_construct.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/throw_exception.hpp>
#include <boost/config.hpp>

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <type_traits>

namespace sleip
{
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
  T*          data_ = nullptr;
  std::size_t size_ = 0;

public:
  dynamic_array() noexcept(noexcept(Allocator())) = default;

  explicit dynamic_array(const Allocator& alloc) noexcept
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
  }

  dynamic_array(size_type count, T const& value, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), count);

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, count, std::addressof(value), 1);
    size_ = count;
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, count, std::addressof(value), 1);
      size_ = count;
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      throw;
    }
#endif
  }

  explicit dynamic_array(size_type count, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), count);

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, count);
    size_ = count;
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, count);
      size_ = count;
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      throw;
    }
#endif
  }

  // impose Forward over Input because we can't resize the allocation so we need to know the
  // range's size up-front
  template <class ForwardIterator>
  dynamic_array(ForwardIterator first, ForwardIterator last, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    auto const count = static_cast<size_type>(std::distance(first, last));

    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), count);

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, count, first);
    size_ = count;
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, count, first);
      size_ = count;
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, count);
      throw;
    }
#endif
  }

  dynamic_array(dynamic_array const& other)
    : boost::empty_value<Allocator, 0>(
        boost::empty_init_t{},
        std::allocator_traits<allocator_type>::select_on_container_copy_construction(
          other.get_allocator()))
  {
    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), other.size());

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, other.size(), other.begin());
    size_ = other.size();
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, other.size(), other.begin());
      size_ = other.size();
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size());
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size());
      throw;
    }
#endif
  }

  dynamic_array(dynamic_array const& other, Allocator const& alloc)
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), other.size());

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, other.size(), other.begin());
    size_ = other.size();
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, other.size(), other.begin());
      size_ = other.size();
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size());
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size());
      throw;
    }
#endif
  }

  dynamic_array(dynamic_array&& other) noexcept
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, get_allocator())
    , data_(other.data_)
    , size_(other.size_)
  {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  dynamic_array(dynamic_array&& other, Allocator const& alloc)
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    if (get_allocator() == other.get_allocator()) {
      data_ = other.data_;
      size_ = other.size_;

      other.data_ = nullptr;
      other.size_ = 0;
      return;
    }

    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), other.size_);

#ifdef BOOST_NO_EXCEPTIONS
    for (size_type idx = 0; idx < other.size_; ++idx) {
      std::allocator_traits<Allocator>::construct(get_allocator(), data_,
                                                  std::move(other.data_[idx]));
    }
#else
    auto idx = size_type{0};
    try {
      for (; idx < other.size_; ++idx) {
        std::allocator_traits<Allocator>::construct(get_allocator(), data_,
                                                    std::move_if_noexcept(other.data_[idx]));
      }
    }
    catch (std::exception const& ex) {
      for (size_type jdx = 0; jdx <= idx; ++jdx) {
        std::allocator_traits<Allocator>::destroy(get_allocator(), data_ + idx - jdx);
      }
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size_);
      boost::throw_exception(ex);
    }
    catch (...) {
      for (size_type jdx = 0; jdx <= idx; ++jdx) {
        std::allocator_traits<Allocator>::destroy(get_allocator(), data_ + idx - jdx);
      }
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, other.size_);
      throw;
    }
#endif
  }

  dynamic_array(std::initializer_list<T> init, Allocator const& alloc = Allocator())
    : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
  {
    data_ = std::allocator_traits<Allocator>::allocate(get_allocator(), init.size());

#ifdef BOOST_NO_EXCEPTIONS
    boost::alloc_construct_n(get_allocator(), data_, init.size(), init.begin());
    size_ = init.size();
#else
    try {
      boost::alloc_construct_n(get_allocator(), data_, init.size(), init.begin());
      size_ = init.size();
    }
    catch (std::exception const& ex) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, init.size());
      boost::throw_exception(ex);
    }
    catch (...) {
      std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, init.size());
      throw;
    }
#endif
  }

  ~dynamic_array()
  {
    boost::alloc_destroy_n(get_allocator(), data_, size_);
    std::allocator_traits<Allocator>::deallocate(get_allocator(), data_, size_);
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
}; // namespace sleip
} // namespace sleip

#endif // SLEIP_DYNAMIC_ARRAY_HPP_
