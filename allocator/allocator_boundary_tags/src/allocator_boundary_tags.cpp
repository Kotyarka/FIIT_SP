#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    throw not_implemented("allocator_boundary_tags::~allocator_boundary_tags()", "your code should be here...");
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&) noexcept", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&) noexcept", "your code should be here...");
}


/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(size_t,std::pmr::memory_resource *,logger *,allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    throw not_implemented("[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t)", "your code should be here...");
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    throw not_implemented("void allocator_boundary_tags::do_deallocate_sm(void *)", "your code should be here...");
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    throw not_implemented("inline void allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode)", "your code should be here...");
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(get_mutex());
    return get_blocks_info_inner();
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> infoVec;

    for (auto it = begin(); it != end(); ++it) {
        infoVec.push_back({it.size(), it.occupied()});
    }

    return infoVec;

}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)", "your code should be here...");
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    throw not_implemented("bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept", "your code should be here...");
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr && _occupied == other._occupied;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept 
{
    return _occupied_ptr != other._occupied_ptr || _occupied != other._occupied;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)", "your code should be here...");
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    return read_block_size(_occupied_ptr);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void *allocator_boundary_tags::boundary_iterator::operator*() const noexcept {
    if (_occupied) {
        return reinterpret_cast<char *>(_occupied) + occupied_block_metadata_size;
    }
    return nullptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted) {
    void *first = read_first_free(_trusted_memory);
    void *pool_start =
        reinterpret_cast<std::byte *>(trusted) + allocator_metadata_size;

    _occupied_ptr = first;
    _occupied = (first != nullptr && first == pool_start);
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}

std::pmr::memory_resource* allocator_boundary_tags::get_parent_allocator() const noexcept {
    return *reinterpret_cast<std::pmr::memory_resource **>(reinterpret_cast<char*>(_trusted_memory));
}

allocator_boundary_tags::fit_mode allocator_boundary_tags::get_fitmode() const noexcept {
    return *reinterpret_cast<allocator_boundary_tags::fit_mode*>(reinterpret_cast<char*>(_trusted_memory) + sizeof(std::pmr::memory_resource*));
}

size_t allocator_boundary_tags::read_space_size(void* trusted) noexcept {
    return *reinterpret_cast<size_t*>(reinterpret_cast<char*>(trusted) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
}
std::mutex& allocator_boundary_tags::get_mutex() const noexcept {
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<char*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t));
}

void* allocator_boundary_tags::read_first_free(void *trusted) noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<char*>(trusted) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));
}

void* allocator_boundary_tags::read_block_back(void *block) noexcept {
    return *reinterpret_cast<void**>(block);
}

void* allocator_boundary_tags::read_block_forward(void *block) noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<char *>(block) + sizeof(void*));
}

size_t allocator_boundary_tags::read_block_size(void *block) noexcept {
    return *reinterpret_cast<size_t *>(reinterpret_cast<char *>(block) + sizeof(void*) + sizeof(void*));
}

void* allocator_boundary_tags::read_block_parent(void *block) noexcept {
    return *reinterpret_cast<void **>(reinterpret_cast<char *>(block) + sizeof(void*) * 2 + sizeof(size_t));
}

size_t allocator_boundary_tags::get_space_size() const noexcept {
    return read_space_size(_trusted_memory);
}

void* allocator_boundary_tags::get_first_free() const noexcept {
    return read_first_free(_trusted_memory);
}