#ifndef SLEIP_DYNAMIC_ARRAY_FWD_HPP_
#define SLEIP_DYNAMIC_ARRAY_FWD_HPP_

// so we can have _HAS_CXX17
//
#if __has_include(<vcruntime.h>)
#include <vcruntime.h>
#endif

#include <boost/config.hpp>

#if !__has_include(<memory_resource>) || (!BOOST_MSVC &&__cplusplus < 201703L) || (BOOST_MSVC && !_HAS_CXX17)
#define SLEIP_NO_CXX17_PMR
#endif

#include <memory>
#ifndef SLEIP_NO_CXX17_PMR
#include <memory_resource>
#endif

namespace sleip
{
template <class T, class Allocator = std::allocator<T>>
struct dynamic_array;

#ifndef SLEIP_NO_CXX17_PMR
namespace pmr
{
template <class T>
using dynamic_array = ::sleip::dynamic_array<T, std::pmr::polymorphic_allocator<T>>;
} // namespace pmr
#endif

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
