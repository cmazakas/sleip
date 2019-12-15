#ifndef SLEIP_DYNAMIC_ARRAY_FWD_HPP_
#define SLEIP_DYNAMIC_ARRAY_FWD_HPP_

#include <boost/core/noinit_adaptor.hpp>

#if __has_include(<memory_resource>)
#include <memory_resource>
#define SLEIP_HAS_PMR 1
#else
#define SLEIP_HAS_PMR 0
#endif

#include <memory>

namespace sleip
{
template <class T, class Allocator = std::allocator<T>>
struct dynamic_array;

template <class T, class Allocator = std::allocator<T>>
using dynamic_array_noinit = dynamic_array<T, boost::noinit_adaptor<Allocator>>;

#ifdef SLEIP_HAS_PMR
namespace pmr
{
template <class T>
using dynamic_array = ::sleip::dynamic_array<T, std::pmr::polymorphic_allocator<T>>;

template <class T>
using dynamic_array_noinit =
  ::sleip::dynamic_array<T, boost::noinit_adaptor<std::pmr::polymorphic_allocator<T>>>;
} // namespace pmr
#endif
#undef SLEIP_HAS_PMR

template <class T, class Allocator>
auto
operator==(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;

template <class T, class Allocator>
auto
operator!=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;

template <class T, class Allocator>
auto
operator<(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;

template <class T, class Allocator>
auto
operator>(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;

template <class T, class Allocator>
auto
operator<=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;

template <class T, class Allocator>
auto
operator>=(dynamic_array<T, Allocator> const& lhs, dynamic_array<T, Allocator> const& rhs) -> bool;
} // namespace sleip

#endif // SLEIP_DYNAMIC_ARRAY_FWD_HPP_
