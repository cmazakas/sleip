# Dynamic Array

`sleip::dynamic_array` is An [AllocatorAwareContainer](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer)
that represents a dynamically-allocated fixed-size buffer. It is a combination of the constructors
for [`std::vector`](https://en.cppreference.com/w/cpp/container/vector)
with the member functions from [`std::array`](https://en.cppreference.com/w/cpp/container/array).

Unlike many containers in the STL, `sleip::dynamic_array` supports array objects and has a `noinit`
constructor overload that enables [default initialization](https://en.cppreference.com/w/cpp/language/default_initialization)
of its underlying elements.

The `dynamic_array` is useful wherever a user may need a managed dynamic allocation where they
won't need resizing or `push_back`-like functionality.
Because it supports [Allocators](https://en.cppreference.com/w/cpp/named_req/Allocator) and can
default-initialize its elements, `dynamic_array` is suitable for performance-oriented applications.

## Example

```c++
#include <sleip/dynamic_array.hpp>

auto buff = sleip::dynamic_array<char>(480 * 640 * 3, sleip::noinit);
auto file = std::fstream("file.dat", std::ios::in | std::ios::binary);
file.read(buff.data(), buff.size());
```
