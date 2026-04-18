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
    throw not_implemented("allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&) noexcept", "your code should be here...");
}

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    throw not_implemented("allocator_sorted_list::allocator_sorted_list(size_t, std::pmr::memory_resource *,logger *,allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{
    throw not_implemented("[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t)", "your code should be here...");
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
{
    throw not_implemented("allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)", "your code should be here...");
}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    throw not_implemented("allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)", "your code should be here...");
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    throw not_implemented("bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept", "your code should be here...");
}

void allocator_sorted_list::do_deallocate_sm(
    void *at)
{
    throw not_implemented("void allocator_sorted_list::do_deallocate_sm(void *)", "your code should be here...");
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    throw not_implemented("inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode)", "your code should be here...");
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

void* allocator_sorted_list::*read_block_next(void *block) noexcept {
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
