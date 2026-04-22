#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    get_mutex().~mutex();
    get_parent_allocator()->deallocate(_trusted_memory, allocator_metadata_size + block_metadata_size + get_space_size());
    _trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (*this == other) {
        return *this;
    }

    get_mutex().~mutex();
    get_parent_allocator()->deallocate(_trusted_memory, allocator_metadata_size + block_metadata_size + get_space_size());
    _trusted_memory = other._trusted_memory;
        // other._trusted_memory = nullptr;
    return *this;


}

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{

    if (parent_allocator == nullptr) {
        parent_allocator = std::pmr::get_default_resource();
    }
    _trusted_memory = parent_allocator->allocate(allocator_metadata_size + block_metadata_size + space_size);
    auto ptr = reinterpret_cast<char *>(_trusted_memory);

    *reinterpret_cast<std::pmr::memory_resource **>(ptr) = parent_allocator;
    ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<fit_mode *>(ptr) = allocate_fit_mode;
    ptr += sizeof(fit_mode);

    *reinterpret_cast<size_t *>(ptr) = space_size;
    ptr += sizeof(size_t);

    new (ptr) std::mutex();
    ptr += sizeof(std::mutex);

    void *first_free = ptr + sizeof(void*);
    *reinterpret_cast<void **>(ptr) = first_free;
    ptr += sizeof(void *);

    *reinterpret_cast<void **>(ptr) = nullptr;
    ptr += sizeof(void *);

    *reinterpret_cast<size_t *>(ptr) = space_size; // ?

}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{
    if (size == 0) {
        return;
    }

    char* current = reinterpret_cast<char*>(get_first_free());
    char* previous = nullptr;

    char* chosen = nullptr;
    char* chosen_prev = nullptr;
    fit_mode fitmode = get_fitmode();
    while (current != nullptr) {
        if (read_block_size(current) >= size) {
        switch (fitmode) {
            case fit_mode::first_fit:
                chosen = current;
                chosen_prev = previous;
                current = nullptr;
                break;
            case fit_mode::the_best_fit:
                if (chosen == nullptr || read_block_size(current) < read_block_size(chosen)) { // sdelat size kak perem
                    chosen = current;
                    chosen_prev = previous;
                }
                break;
            case fit_mode::the_worst_fit:
                if (chosen == nullptr || read_block_size(current) > read_block_size(chosen)) {
                    chosen = current;
                    chosen_prev = previous;
                }
                break;
        } 
    }
        if (current != nullptr) {
            previous = current;
            current = reinterpret_cast<char*>(read_block_next(current));
        }
    }
    if (chosen == nullptr) {
        throw std::bad_alloc();
    }

    size_t chosen_size = read_block_size(chosen);
    void* remainder = nullptr;

    if (chosen_size > block_metadata_size + size + 1) {
        remainder = chosen + size + block_metadata_size;
        *reinterpret_cast<void* *>(remainder) = read_block_next(current);
        *reinterpret_cast<size_t *>(reinterpret_cast<char*>(remainder) + sizeof(void*)) = (read_block_size(chosen) - block_metadata_size - size);
    }

    void* next = remainder != nullptr ? remainder : read_block_next(chosen);
    
    if (chosen_prev == nullptr) {
        set_first_free(previous);
    } else {
        *reinterpret_cast<void**>(chosen_prev) = next;
    }

    *reinterpret_cast<size_t*>(reinterpret_cast<char*>(chosen)+sizeof(void*)) = size;

    return reinterpret_cast<char*>(chosen) + block_metadata_size;
}


allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
{
    size_t totalSize =
        allocator_metadata_size + block_metadata_size + other.get_space_size();
    _trusted_memory = other.get_parent_allocator()->allocate(totalSize);
    std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
    new (&get_mutex()) std::mutex();
}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    if (*this == other)
        return *this;

    get_mutex().~mutex();
    get_parent_allocator()->deallocate(_trusted_memory, allocator_metadata_size +
                                                    block_metadata_size +
                                                    get_space_size());

    size_t totalSize =
        allocator_metadata_size + block_metadata_size + other.get_space_size();
    _trusted_memory = other.get_parent_allocator()->allocate(totalSize);
    std::memcpy(_trusted_memory, other._trusted_memory, totalSize);
    new (&get_mutex()) std::mutex();

    return *this;
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
  auto *o = dynamic_cast<const allocator_sorted_list *>(&other);
  return o != nullptr && o->_trusted_memory == _trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(
    void *at)
{
    throw not_implemented("void allocator_sorted_list::do_deallocate_sm(void *)", "your code should be here...");
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
                sizeof(std::pmr::memory_resource *);
    *reinterpret_cast<fit_mode *>(ptr) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{   
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept", "your code should be here...");
}


std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept", "your code should be here...");
}


bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    throw not_implemented("bool allocator_sorted_list::sorted_free_iterator::operator==(const allocator_sorted_list::sorted_free_iterator &) const noexcept", "your code should be here...");
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    throw not_implemented("bool allocator_sorted_list::sorted_free_iterator::operator!=(const allocator_sorted_list::sorted_free_iterator &) const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int)", "your code should be here...");
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    throw not_implemented("size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept", "your code should be here...");
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    throw not_implemented("void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()", "your code should be here...");
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)
{
    throw not_implemented("allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *)", "your code should be here...");
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    throw not_implemented("bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator &) const noexcept", "your code should be here...");
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    throw not_implemented("bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &) const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    throw not_implemented("allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    throw not_implemented("allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int)", "your code should be here...");
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    throw not_implemented("size_t allocator_sorted_list::sorted_iterator::size() const noexcept", "your code should be here...");
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    throw not_implemented("void *allocator_sorted_list::sorted_iterator::operator*() const noexcept", "your code should be here...");
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
{
    throw not_implemented("allocator_sorted_list::sorted_iterator::sorted_iterator() ", "your code should be here...");
}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
{
    throw not_implemented("allocator_sorted_list::sorted_iterator::sorted_iterator(void *)", "your code should be here...");
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    throw not_implemented("bool allocator_sorted_list::sorted_iterator::occupied() const noexcept", "your code should be here...");
}

std::pmr::memory_resource* allocator_sorted_list::get_parent_allocator() const noexcept {
    return *reinterpret_cast<std::pmr::memory_resource **>(reinterpret_cast<char*>(_trusted_memory));
}

allocator_sorted_list::fit_mode allocator_sorted_list::get_fitmode() const noexcept {
    return *reinterpret_cast<allocator_sorted_list::fit_mode*>(reinterpret_cast<char*>(_trusted_memory) + sizeof(std::pmr::memory_resource*));
}

size_t allocator_sorted_list::read_space_size(void* trusted) noexcept {
    return *reinterpret_cast<size_t*>(reinterpret_cast<char*>(trusted) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
}
std::mutex& allocator_sorted_list::get_mutex() const noexcept {
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<char*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t));
}

void* allocator_sorted_list::read_first_free(void *trusted) noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<char*>(trusted) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));
}

void* allocator_sorted_list::read_block_next(void *block) noexcept {
    return *reinterpret_cast<void**>(block);
}

size_t allocator_sorted_list::read_block_size(void *block) noexcept {
    return *reinterpret_cast<size_t *>(reinterpret_cast<char *>(block) + sizeof(void*));
}

size_t allocator_sorted_list::get_space_size() const noexcept {
    return read_space_size(_trusted_memory);
}

void* allocator_sorted_list::get_first_free() const noexcept {
    return read_first_free(_trusted_memory);
}
