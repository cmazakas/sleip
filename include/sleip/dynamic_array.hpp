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

    auto idx = size_type{};

#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif
      auto const count = other.size();

      auto d = std::unique_ptr<T[], dealloc>(
        std::allocator_traits<Allocator>::allocate(alloc_, count), dealloc(alloc_, count));

      for (idx = 0; idx < other.size(); ++idx) {
        std::allocator_traits<Allocator>::construct(alloc_, d.get(),
                                                    std::move_if_noexcept(other.data_[idx]));
      }

      data_ = d.release();
      size_ = count;

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      for (size_type jdx = 0; jdx <= idx; ++jdx) {
        std::allocator_traits<Allocator>::destroy(alloc_, data() + idx - jdx);
      }
      boost::throw_exception(ex);
    }
    catch (...) {
      for (size_type jdx = 0; jdx <= idx; ++jdx) {
        std::allocator_traits<Allocator>::destroy(alloc_, data() + idx - jdx);
      }
      throw;
    }
#endif
  }

  dynamic_array(std::initializer_list<T> init, Allocator const& alloc = Allocator())
    : dynamic_array(init.begin(), init.end(), alloc)
  {
  }

  ~dynamic_array()
  {
    auto& alloc = boost::empty_value<Allocator, 0>::get();
    boost::alloc_destroy_n(alloc, data_, size_);
    std::allocator_traits<Allocator>::deallocate(alloc, data_, size_);
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
