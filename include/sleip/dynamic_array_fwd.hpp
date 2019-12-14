#ifndef SLEIP_DYNAMIC_ARRAY_FWD_HPP_
#define SLEIP_DYNAMIC_ARRAY_FWD_HPP_

#include <boost/core/noinit_adaptor.hpp>
#include <memory>

namespace sleip
{
template <class T, class Allocator = std::allocator<T>>
struct dynamic_array;

template <class T, class Allocator = std::allocator<T>>
using dynamic_array_noinit = dynamic_array<T, boost::noinit_adaptor<Allocator>>;

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
