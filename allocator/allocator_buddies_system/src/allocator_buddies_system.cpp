#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"
#include <cstring>
#include <iostream>

static inline void* ptr_add(void* ptr, size_t bytes) {
    return reinterpret_cast<char*>(ptr) + bytes;
}

static inline ptrdiff_t ptr_diff(void* ptr1, void* ptr2) {
    return reinterpret_cast<char*>(ptr1) - reinterpret_cast<char*>(ptr2);
}

static inline std::pmr::memory_resource*& get_parent(void* trusted) {
    return *reinterpret_cast<std::pmr::memory_resource**>(trusted);
}

static inline allocator_with_fit_mode::fit_mode& get_fit_mode(void* trusted) {
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
        ptr_add(trusted, sizeof(std::pmr::memory_resource*)));
}

static inline unsigned char& get_k_max(void* trusted) {
    return *reinterpret_cast<unsigned char*>(
        ptr_add(trusted, sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode)));
}

static inline std::mutex& get_mutex(void* trusted) { 
    size_t offset = sizeof(std::pmr::memory_resource*) + 
                    sizeof(allocator_with_fit_mode::fit_mode) + 
                    sizeof(unsigned char);
    offset = (offset + alignof(std::mutex) - 1) & ~(alignof(std::mutex) - 1);
    return *reinterpret_cast<std::mutex*>(ptr_add(trusted, offset));
}

static inline void* get_data_start(void* trusted) {
    size_t offset = sizeof(std::pmr::memory_resource*) + 
                    sizeof(allocator_with_fit_mode::fit_mode) + 
                    sizeof(unsigned char);
    offset = (offset + alignof(std::mutex) - 1) & ~(alignof(std::mutex) - 1);
    offset += sizeof(std::mutex);
    return ptr_add(trusted, offset);
}

static inline void* get_data_end(void* trusted) {
    unsigned char k_max = get_k_max(trusted);
    void* data_start = get_data_start(trusted);
    return ptr_add(data_start, 1ULL << k_max); //1ULL << k_max — это 2^k_max 
}

static inline void*& get_free_list_head(void* trusted) {
    void* data_start = get_data_start(trusted);
    return *reinterpret_cast<void**>(data_start);
}

static inline void* get_first_block_start(void* trusted) {
    return ptr_add(get_data_start(trusted), sizeof(void*));
}

static inline void*& get_next_free(void* block) {
    return *reinterpret_cast<void**>(ptr_add(block, 1)); 
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size_power_of_two,
    std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size_power_of_two < 2) {
        throw std::logic_error("allocator_buddies_system: space_size_power_of_two must be at least 2");
    }

    size_t actual_size = 1;
    int k_max = 0;
    while (actual_size < space_size_power_of_two) {
        actual_size <<= 1;
        k_max++;
    }
    
    size_t min_required = 1ULL << allocator_buddies_system::min_k;
    if (actual_size < min_required) {
        actual_size = min_required;
        k_max = allocator_buddies_system::min_k;
    }
    
    std::pmr::memory_resource* alloc = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
    
    size_t metadata_size = sizeof(std::pmr::memory_resource*) + 
                           sizeof(allocator_with_fit_mode::fit_mode) + 
                           sizeof(unsigned char);
    size_t mutex_alignment = alignof(std::mutex);
    size_t aligned_metadata = (metadata_size + mutex_alignment - 1) & ~(mutex_alignment - 1);
    aligned_metadata += sizeof(std::mutex);
    
    size_t total_size = aligned_metadata + sizeof(void*) + actual_size;
    
    _trusted_memory = alloc->allocate(total_size, alignof(std::max_align_t));
    if (!_trusted_memory) throw std::bad_alloc();
 
    get_parent(_trusted_memory) = alloc;
    get_fit_mode(_trusted_memory) = allocate_fit_mode;
    get_k_max(_trusted_memory) = static_cast<unsigned char>(k_max);
    
    new (&get_mutex(_trusted_memory)) std::mutex();
    
    void* data_start = get_data_start(_trusted_memory);
    void* first_block = get_first_block_start(_trusted_memory); 
    
    get_free_list_head(_trusted_memory) = nullptr;
    
    block_metadata* meta = reinterpret_cast<block_metadata*>(first_block);
    meta->occupied = false;
    meta->size = static_cast<unsigned char>(k_max);
    
    get_next_free(first_block) = nullptr;
    get_free_list_head(_trusted_memory) = first_block;
}

allocator_buddies_system::~allocator_buddies_system()
{
    if (_trusted_memory) {
        std::pmr::memory_resource* parent = get_parent(_trusted_memory);
        unsigned char k_max = get_k_max(_trusted_memory);
        
        size_t metadata_size = sizeof(std::pmr::memory_resource*) + 
                               sizeof(allocator_with_fit_mode::fit_mode) + 
                               sizeof(unsigned char);
        size_t mutex_alignment = alignof(std::mutex);
        size_t aligned_metadata = (metadata_size + mutex_alignment - 1) & ~(mutex_alignment - 1);
        aligned_metadata += sizeof(std::mutex);

        size_t total_size = aligned_metadata + sizeof(void*) + (1ULL << k_max);
        
        get_mutex(_trusted_memory).~mutex();
        parent->deallocate(_trusted_memory, total_size, alignof(std::max_align_t));
        _trusted_memory = nullptr;
    }
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
{
    if (!other._trusted_memory) {
        _trusted_memory = nullptr;
        return;
    }
    
    std::lock_guard<std::mutex> lock(get_mutex(other._trusted_memory));

    std::pmr::memory_resource* parent = get_parent(other._trusted_memory);
    fit_mode mode = get_fit_mode(other._trusted_memory);
    unsigned char k_max = get_k_max(other._trusted_memory);
    size_t space_size = 1ULL << k_max;
    
    new (this) allocator_buddies_system(space_size, parent, mode);
    
    void* first_block_other = get_first_block_start(other._trusted_memory);
    void* first_block_this = get_first_block_start(_trusted_memory);
    size_t space_size_bytes = 1ULL << k_max;
    std::memcpy(first_block_this, first_block_other, space_size_bytes);
    
    void* other_free = get_free_list_head(other._trusted_memory);
    void** this_prev = &get_free_list_head(_trusted_memory);
    
    while (other_free) {
        ptrdiff_t offset = ptr_diff(other_free, first_block_other);
        void* new_block = ptr_add(first_block_this, offset);
        
        *this_prev = new_block;
        this_prev = &get_next_free(new_block);
        other_free = get_next_free(other_free);
    }
    *this_prev = nullptr;
}

void allocator_buddies_system::swap(allocator_buddies_system& other) noexcept
{
    if (this == &other) return;
    
    std::scoped_lock lock(get_mutex(_trusted_memory), get_mutex(other._trusted_memory));
    std::swap(_trusted_memory, other._trusted_memory);
}

allocator_buddies_system& allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    if (this == &other) return *this;
    
    allocator_buddies_system temp(other);  
    swap(temp); 
    return *this;
}

allocator_buddies_system::allocator_buddies_system(allocator_buddies_system &&other) noexcept
{
    std::lock_guard<std::mutex> lock(get_mutex(other._trusted_memory));
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_buddies_system &allocator_buddies_system::operator=(allocator_buddies_system &&other) noexcept
{
    if (this == &other) return *this;
    
    allocator_buddies_system temp(std::move(other));
    swap(temp);
    
    return *this;
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(size_t size)
{
    if (size == 0) {
        size = 1;
    }
    
    std::lock_guard<std::mutex> lock(get_mutex(_trusted_memory));
    
    unsigned char k_max = get_k_max(_trusted_memory);
    
    size_t required_size = size + allocator_buddies_system::occupied_block_metadata_size;
    int k = __detail::nearest_greater_k_of_2(required_size);

    if (k < allocator_buddies_system::min_k) {
        k = allocator_buddies_system::min_k;
    }
    
    void** prev_ptr = &get_free_list_head(_trusted_memory);
    void* current = *prev_ptr; 
    void* best_block = nullptr;
    void** best_prev_ptr = nullptr; 
    size_t best_size = 0;
    bool found = false;
    fit_mode mode = get_fit_mode(_trusted_memory);
    
    while (current) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(current);
        size_t block_k = meta->size;
        size_t block_size = 1ULL << block_k;
        
        if (block_k >= k) {
            bool is_better = false;
            switch (mode) {
                case fit_mode::first_fit:
                    if (!found) is_better = true;
                    break;
                case fit_mode::the_best_fit:
                    if (!found || block_size < best_size) is_better = true;
                    break;
                case fit_mode::the_worst_fit:
                    if (!found || block_size > best_size) is_better = true;
                    break;
            }
            
            if (is_better) {
                best_block = current;
                best_prev_ptr = prev_ptr;
                best_size = block_size;
                found = true;
                if (mode == fit_mode::first_fit) break;
            }
        }
        
        prev_ptr = &get_next_free(current); 
        current = *prev_ptr;
    }
    
    if (!found) {
        throw std::bad_alloc();
    }
    
    *best_prev_ptr = get_next_free(best_block);
    
    int current_k = static_cast<int>(reinterpret_cast<block_metadata*>(best_block)->size);
    void* block = best_block;
    
    while (current_k > k) {
        current_k--;
        size_t half_size = 1ULL << current_k;
        void* buddy = ptr_add(block, half_size);
        
        block_metadata* block_meta = reinterpret_cast<block_metadata*>(block);
        block_meta->size = static_cast<unsigned char>(current_k);
        
        block_metadata* buddy_meta = reinterpret_cast<block_metadata*>(buddy);
        buddy_meta->occupied = false;
        buddy_meta->size = static_cast<unsigned char>(current_k);
        
        get_next_free(buddy) = get_free_list_head(_trusted_memory);
        get_free_list_head(_trusted_memory) = buddy;
    }
    
    block_metadata* meta = reinterpret_cast<block_metadata*>(block);
    meta->occupied = true;
    meta->size = static_cast<unsigned char>(k);
    
    void* parent_ptr = ptr_add(block, sizeof(block_metadata));
    *reinterpret_cast<void**>(parent_ptr) = this;
    
    return ptr_add(block, allocator_buddies_system::occupied_block_metadata_size);
}

void allocator_buddies_system::do_deallocate_sm(void* at)
{
    if (!at) return;
    
    std::lock_guard<std::mutex> lock(get_mutex(_trusted_memory));
    
    void* block = ptr_add(at, -static_cast<ptrdiff_t>(allocator_buddies_system::occupied_block_metadata_size));
    
    block_metadata* meta = reinterpret_cast<block_metadata*>(block);
    
    if (!meta->occupied) {
        throw std::invalid_argument("allocator_buddies_system::do_deallocate_sm: block already free");
    }
    
    void* parent_ptr = ptr_add(block, sizeof(block_metadata));
    if (*reinterpret_cast<void**>(parent_ptr) != this) {
        throw std::invalid_argument("allocator_buddies_system::do_deallocate_sm: block does not belong to this allocator");
    }
    
    unsigned char k_max = get_k_max(_trusted_memory);
    void* data_start = get_data_start(_trusted_memory);
    void* data_end = get_data_end(_trusted_memory);
    
    int k = meta->size;
    meta->occupied = false;
    
    while (k < k_max) {
        void* first_block = get_first_block_start(_trusted_memory); 
        uintptr_t base = reinterpret_cast<uintptr_t>(first_block);
        uintptr_t offset = reinterpret_cast<uintptr_t>(block) - base;
        uintptr_t buddy_offset = offset ^ (1ULL << k);
        void* buddy = reinterpret_cast<void*>(base + buddy_offset);
        
        if (buddy < data_start || buddy >= data_end) break;
        
        block_metadata* buddy_meta = reinterpret_cast<block_metadata*>(buddy);
        if (buddy_meta->occupied || buddy_meta->size != k) break;
        
        void** prev_ptr = &get_free_list_head(_trusted_memory);
        void* curr = *prev_ptr;
        while (curr && curr != buddy) {
            prev_ptr = &get_next_free(curr);
            curr = *prev_ptr;
        }
        if (curr == buddy) {
            *prev_ptr = get_next_free(curr);
        }
        
        if (buddy_offset < offset) {
            block = buddy;
        }
        k++;
        
        meta = reinterpret_cast<block_metadata*>(block);
        meta->occupied = false;
        meta->size = static_cast<unsigned char>(k);
    }
    
    get_next_free(block) = get_free_list_head(_trusted_memory);
    get_free_list_head(_trusted_memory) = block;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    const allocator_buddies_system* other_alloc = dynamic_cast<const allocator_buddies_system*>(&other);
    if (!other_alloc) return false;
    return _trusted_memory == other_alloc->_trusted_memory;
}

inline void allocator_buddies_system::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(get_mutex(_trusted_memory));
    get_fit_mode(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    try {
        return get_blocks_info_inner();
    } catch (...) {
        return std::vector<allocator_test_utils::block_info>();
    }
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;
    
    std::lock_guard<std::mutex> lock(get_mutex(_trusted_memory));
    
    void* current = get_first_block_start(_trusted_memory);
    void* end = get_data_end(_trusted_memory);
    
    while (current < end) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(current);
        size_t block_size = 1ULL << meta->size;
        
        allocator_test_utils::block_info info;
        info.block_size = block_size;
        info.is_block_occupied = meta->occupied;
        result.push_back(info);
        
        current = ptr_add(current, block_size);
    }
    
    return result;
}

allocator_buddies_system::buddy_iterator::buddy_iterator()
    : _block(nullptr) {}

allocator_buddies_system::buddy_iterator::buddy_iterator(void* start)
    : _block(start) {}

bool allocator_buddies_system::buddy_iterator::operator==(const buddy_iterator& other) const noexcept {
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const buddy_iterator& other) const noexcept {
    return _block != other._block;
}

allocator_buddies_system::buddy_iterator& allocator_buddies_system::buddy_iterator::operator++() & noexcept {
    if (_block) {
        block_metadata* meta = reinterpret_cast<block_metadata*>(_block);
        size_t block_size = 1ULL << meta->size;
        _block = ptr_add(_block, block_size);
    }
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n) {
    buddy_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept {
    if (!_block) return 0;
    block_metadata* meta = reinterpret_cast<block_metadata*>(_block);
    return 1ULL << meta->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept {
    if (!_block) return false;
    block_metadata* meta = reinterpret_cast<block_metadata*>(_block);
    return meta->occupied;
}

void* allocator_buddies_system::buddy_iterator::operator*() const noexcept {
    if (!_block) return nullptr;
    return ptr_add(_block, allocator_buddies_system::occupied_block_metadata_size);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept {
    return buddy_iterator(get_first_block_start(_trusted_memory));
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept {
    return buddy_iterator(get_data_end(_trusted_memory));
}