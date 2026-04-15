#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap()
    :   smart_mem_resource()
{
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (size == 0) {
        return nullptr;
    }

    void* ptr = ::operator new(size, std::nothrow);

    if (ptr == nullptr) {
        throw std::bad_alloc();
    }

    return ptr;


}

void allocator_global_heap::do_deallocate_sm(
    void *at)
{
    std::lock_guard<std::mutex> lock(mutex);
    ::operator delete(at);
}

allocator_global_heap::~allocator_global_heap()
{
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other) : smart_mem_resource(other)
{
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other)
{
    return *this;
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    std::lock_guard<std::mutex> lock(mutex);
    return this == &other;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept
    : smart_mem_resource(std::move(other))
{
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    if (this != &other) {
        std::lock_guard<std::mutex> lock(mutex);
        smart_mem_resource::operator=(std::move(other));
    }
    return *this;
}
