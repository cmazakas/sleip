#include <sleip/dynamic_array.hpp>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include <boost/core/lightweight_test.hpp>

namespace ipc = boost::interprocess;

void
test_shmem_allocator()
{
  // Remove shared memory on construction and destruction
  struct shm_remove
  {
    shm_remove() { ipc::shared_memory_object::remove("MySharedMemory"); }
    ~shm_remove() { ipc::shared_memory_object::remove("MySharedMemory"); }
  } remover;

  // Create shared memory
  auto segment = ipc::managed_shared_memory(ipc::create_only,
                                            "MySharedMemory", // segment name
                                            65536);

  // Create an allocator that allocates ints from the managed segment
  auto allocator_instance =
    ipc::allocator<int, ipc::managed_shared_memory::segment_manager>(segment.get_segment_manager());

  static_assert(
    !std::is_same_v<typename std::allocator_traits<ipc::allocator<
                      int, ipc::managed_shared_memory::segment_manager>>::value_type*,
                    typename std::allocator_traits<
                      ipc::allocator<int, ipc::managed_shared_memory::segment_manager>>::pointer>,
    "Must test fancy ::pointer types!");

  auto a =
    sleip::dynamic_array<int, ipc::allocator<int, ipc::managed_shared_memory::segment_manager>>(
      std::size_t{128}, -1, allocator_instance);

  auto b =
    sleip::dynamic_array<int, ipc::allocator<int, ipc::managed_shared_memory::segment_manager>>(
      std::size_t{128}, -1, allocator_instance);

  BOOST_TEST((a.get_allocator() == allocator_instance));
  BOOST_TEST((b.get_allocator() == allocator_instance));

  BOOST_TEST_EQ(a.size(), 128);
  BOOST_TEST_EQ(b.size(), a.size());

  BOOST_TEST_ALL_EQ(a.begin(), a.end(), b.begin(), b.end());
}

int
main()
{
  test_shmem_allocator();

  return boost::report_errors();
}
