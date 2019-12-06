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
struct dynamic_array
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
  struct deleter : boost::empty_value<Allocator, 0>
  {
    std::size_t size = 0;

    deleter() noexcept(noexcept(Allocator()))
      : boost::empty_value<Allocator, 0>(boost::empty_init_t{})
    {
    }

    deleter(deleter const&) = delete;

    deleter(deleter&& other) noexcept
      : boost::empty_value<Allocator, 0>(
          boost::empty_init_t{},
          std::move(static_cast<boost::empty_value<Allocator, 0>&>(other).get()))
      , size{other.size}
    {
      other.size = 0;
    };

    deleter(std::size_t size_)
      : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, Allocator())
      , size{size_}
    {
    }

    deleter(Allocator const& alloc, std::size_t size_)
      : boost::empty_value<Allocator, 0>(boost::empty_init_t{}, alloc)
      , size{size_}
    {
    }

    auto
    operator=(deleter const&) & -> deleter& = delete;

    auto
      operator=(deleter&& other) &
      noexcept -> deleter&
    {
      boost::empty_value<Allocator, 0>::get() =
        std::move(static_cast<boost::empty_value<Allocator, 0>&>(other).get());

      size       = other.size;
      other.size = 0;

      return *this;
    }

    auto
    get_allocator() const noexcept -> Allocator
    {
      return boost::empty_value<Allocator, 0>::get();
    }

    auto
    operator()(T* ptr) -> void
    {
      auto alloc = get_allocator();
      std::allocator_traits<Allocator>::deallocate(alloc, ptr, size);
    }
  };

  std::unique_ptr<T, deleter> data_ = {nullptr, deleter(Allocator(), 0)};

public:
  dynamic_array() noexcept(noexcept(Allocator())){};

  explicit dynamic_array(const Allocator& alloc) noexcept
    : data_(nullptr, deleter(alloc, 0))
  {
  }

  dynamic_array(size_type count, T const& value, Allocator const& alloc = Allocator())
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif

      auto alloc_ = alloc;
      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, count), deleter(alloc_, count)};
      boost::alloc_construct_n(alloc_, data(), count, std::addressof(value), 1);

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  explicit dynamic_array(size_type count, Allocator const& alloc = Allocator())
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif
      auto alloc_ = alloc;
      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, count), deleter(alloc_, count)};
      boost::alloc_construct_n(alloc_, data(), count);
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  // impose Forward over Input because we can't resize the allocation so we need to know the
  // range's size up-front
  template <class ForwardIterator>
  dynamic_array(ForwardIterator first, ForwardIterator last, Allocator const& alloc = Allocator())
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif
      auto const count = static_cast<size_type>(std::distance(first, last));

      auto alloc_ = alloc;
      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, count), deleter(alloc_, count)};
      boost::alloc_construct_n(alloc_, data(), count, first);

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  dynamic_array(dynamic_array const& other)
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif

      auto alloc = std::allocator_traits<allocator_type>::select_on_container_copy_construction(
        other.get_allocator());

      data_ = {std::allocator_traits<Allocator>::allocate(alloc, other.size()),
               deleter(alloc, other.size())};

      boost::alloc_construct_n(alloc, data(), other.size(), other.begin());

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  dynamic_array(dynamic_array const& other, Allocator const& alloc)
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif

      auto alloc_ = alloc;

      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
               deleter(alloc_, other.size())};

      boost::alloc_construct_n(alloc_, data(), other.size(), other.begin());

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  dynamic_array(dynamic_array&& other) noexcept
    : data_(std::move(other).data_)
  {
  }

  dynamic_array(dynamic_array&& other, Allocator const& alloc)
  {
    auto alloc_ = alloc;

    if (alloc_ == other.get_allocator()) {
      data_ = std::move(other).data_;
      return;
    }

    auto idx = size_type{};
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif

      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, other.size()),
               deleter(alloc_, other.size())};

      for (idx = 0; idx < other.size(); ++idx) {
        std::allocator_traits<Allocator>::construct(alloc_, data(), std::move(other.data()[idx]));
      }

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
  {
#ifndef BOOST_NO_EXCEPTIONS
    try {
#endif

      auto       alloc_ = alloc;
      auto const count  = init.size();

      data_ = {std::allocator_traits<Allocator>::allocate(alloc_, count), deleter(alloc, count)};

      boost::alloc_construct_n(alloc_, data(), init.size(), init.begin());

#ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception const& ex) {
      boost::throw_exception(ex);
    }
#endif
  }

  ~dynamic_array()
  {
    auto alloc = get_allocator();
    boost::alloc_destroy_n(alloc, data(), size());
  }

  auto
  get_allocator() const -> allocator_type
  {
    return data_.get_deleter().get_allocator();
  }

  auto
  size() const noexcept -> size_type
  {
    return data_.get_deleter().size;
  }

  auto
  data() noexcept -> pointer
  {
    return data_.get();
  }

  auto
  data() const noexcept -> const_pointer
  {
    return data_.get();
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
};
} // namespace sleip

#endif // SLEIP_DYNAMIC_ARRAY_HPP_
