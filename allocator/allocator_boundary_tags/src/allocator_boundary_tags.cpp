#include "../include/allocator_boundary_tags.h"
#include <cstring>
#include <stdexcept>

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory)
    {
        auto* parent = get_parent_allocator();
        parent->deallocate(
            _trusted_memory,
            read_space_size(_trusted_memory) + allocator_metadata_size
        );
        _trusted_memory = nullptr;
    }
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
    : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other)
    {
        if (_trusted_memory)
        {
            auto* parent = get_parent_allocator();
            parent->deallocate(
                _trusted_memory,
                read_space_size(_trusted_memory) + allocator_metadata_size
            );
        }

        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (!parent_allocator)
        parent_allocator = std::pmr::get_default_resource();

    _trusted_memory = parent_allocator->allocate(
        space_size + allocator_metadata_size
    );

    char* ptr = reinterpret_cast<char*>(_trusted_memory);

    *reinterpret_cast<std::pmr::memory_resource**>(ptr) = parent_allocator;
    ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<fit_mode*>(ptr) = allocate_fit_mode;
    ptr += sizeof(fit_mode);

    *reinterpret_cast<size_t*>(ptr) = space_size;
    ptr += sizeof(size_t);

    new (ptr) std::mutex;
    ptr += sizeof(std::mutex);

    *reinterpret_cast<void**>(ptr) = nullptr;
}

void *allocator_boundary_tags::do_allocate_sm(size_t size)
{
    std::lock_guard<std::mutex> lock(get_mutex());

    const size_t needed = size + occupied_block_metadata_size;

    char* pool_start =
        reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    char* pool_end = pool_start + get_space_size();

    char* selected = nullptr;
    void* selected_prev = nullptr;
    void* selected_next = nullptr;
    size_t selected_gap = 0;

    if (!get_first_free())
    {
        if (get_space_size() < needed)
            throw std::bad_alloc();

        selected = pool_start;
    }
    else
    {
        char* current = pool_start;
        void* prev = nullptr;

        for (auto it = begin(); it != end(); ++it)
        {
            char* block = reinterpret_cast<char*>(it.get_ptr());
            
            size_t gap = block - current;
            if (gap >= needed)
            {
                switch (get_fitmode())
                {
                    case fit_mode::first_fit:
                        if (!selected)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = block;
                            selected_gap = gap;
                        }
                        break;
                    case fit_mode::the_best_fit:
                        if (!selected || gap < selected_gap)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = block;
                            selected_gap = gap;
                        }
                        break;
                    case fit_mode::the_worst_fit:
                        if (!selected || gap > selected_gap)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = block;
                            selected_gap = gap;
                        }
                        break;
                }
            }

            if (get_fitmode() == fit_mode::first_fit && selected)
                break;

            current = block + read_block_size(block);
            prev = block;
        }

        if (!(get_fitmode() == fit_mode::first_fit && selected))
        {
            size_t gap = pool_end - current;
            if (gap >= needed)
            {
                switch (get_fitmode())
                {
                    case fit_mode::first_fit:
                        if (!selected)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = nullptr;
                            selected_gap = gap;
                        }
                        break;
                    case fit_mode::the_best_fit:
                        if (!selected || gap < selected_gap)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = nullptr;
                            selected_gap = gap;
                        }
                        break;
                    case fit_mode::the_worst_fit:
                        if (!selected || gap > selected_gap)
                        {
                            selected = current;
                            selected_prev = prev;
                            selected_next = nullptr;
                            selected_gap = gap;
                        }
                        break;
                }
            }
        }
    }

    if (!selected)
        throw std::bad_alloc();

    char* block = selected;
    size_t real_size = needed;

    if (selected_gap - needed < occupied_block_metadata_size){
        real_size = selected_gap;
    }
    *reinterpret_cast<size_t*>(block) = real_size;
    *reinterpret_cast<void**>(block + sizeof(size_t)) = selected_prev;
    *reinterpret_cast<void**>(block + sizeof(size_t) + sizeof(void*) * 2) = selected_next;
    *reinterpret_cast<void**>(block + sizeof(size_t) + sizeof(void*) * 3) = this;

    if (selected_prev)
    {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(selected_prev)
            + sizeof(size_t)
            + sizeof(void*) * 2
        ) = block;
    }
    else
    {
        set_first_free(block);
    }

    if (selected_next)
    {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(selected_next)
            + sizeof(size_t)
        ) = block;
    }

    return block + occupied_block_metadata_size;
}

void allocator_boundary_tags::do_deallocate_sm(void *at)
{
    if (!at)
        return;

    std::lock_guard<std::mutex> lock(get_mutex());

    char* block =
        reinterpret_cast<char*>(at) - occupied_block_metadata_size;

    void* prev = read_block_back(block);
    void* next = read_block_forward(block);

    if (prev)
    {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(prev)
            + sizeof(size_t)
            + sizeof(void*) * 2
        ) = next;
    }
    else
    {
        set_first_free(next);
    }

    if (next)
    {
        *reinterpret_cast<void**>(
            reinterpret_cast<char*>(next)
            + sizeof(size_t)
        ) = prev;
    }
}

void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto *ptr = reinterpret_cast<char *>(_trusted_memory) +
                sizeof(std::pmr::memory_resource *);
    *reinterpret_cast<fit_mode *>(ptr) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(get_mutex());
    return get_blocks_info_inner();
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::begin() const noexcept
{
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<block_info> result;

    char* pool_start =
        reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
    char* pool_end = pool_start + get_space_size();

    char* current = pool_start;

    for (auto it = begin(); it != end(); ++it)
    {
        char* block = reinterpret_cast<char*>(it.get_ptr());

        if (block > current)
        {
            result.push_back({
                static_cast<size_t>(block - current),
                false
            });
        }

        result.push_back({
            read_block_size(block),
            true
        });

        current = block + read_block_size(block);
    }

    if (current < pool_end)
    {
        result.push_back({
            static_cast<size_t>(pool_end - current),
            false
        });
    }

    return result;
}

allocator_boundary_tags::allocator_boundary_tags(
    const allocator_boundary_tags &other)
{
    std::lock_guard<std::mutex> lock(other.get_mutex());

    auto* parent = other.get_parent_allocator();
    size_t total = other.get_space_size() + allocator_metadata_size;

    _trusted_memory = parent->allocate(total);
    std::memcpy(_trusted_memory, other._trusted_memory, total);

    char* mutex_ptr =
        reinterpret_cast<char*>(_trusted_memory) +
        sizeof(std::pmr::memory_resource*) +
        sizeof(fit_mode) +
        sizeof(size_t);

    new (mutex_ptr) std::mutex;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    const allocator_boundary_tags &other)
{
    if (this == &other)
        return *this;

    std::lock_guard<std::mutex> lock(other.get_mutex());

    if (_trusted_memory)
    {
        auto* parent = get_parent_allocator();
        parent->deallocate(
            _trusted_memory,
            get_space_size() + allocator_metadata_size
        );
    }

    auto* parent = other.get_parent_allocator();
    size_t total = other.get_space_size() + allocator_metadata_size;

    _trusted_memory = parent->allocate(total);
    std::memcpy(_trusted_memory, other._trusted_memory, total);

    char* mutex_ptr =
        reinterpret_cast<char*>(_trusted_memory) +
        sizeof(std::pmr::memory_resource*) +
        sizeof(fit_mode) +
        sizeof(size_t);

    new (mutex_ptr) std::mutex;

    return *this;
}

bool allocator_boundary_tags::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
    const boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const boundary_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator&
allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied_ptr)
        _occupied_ptr = read_block_forward(_occupied_ptr);

    _occupied = (_occupied_ptr != nullptr);
    return *this;
}

allocator_boundary_tags::boundary_iterator&
allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_occupied_ptr)
        _occupied_ptr = read_block_back(_occupied_ptr);

    _occupied = (_occupied_ptr != nullptr);
    return *this;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator++(int)
{
    boundary_iterator temp = *this;
    ++(*this);
    return temp;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator--(int)
{
    boundary_iterator temp = *this;
    --(*this);
    return temp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    return read_block_size(_occupied_ptr);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void *allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    if (!_occupied)
        return nullptr;

    return reinterpret_cast<char*>(_occupied_ptr) +
           occupied_block_metadata_size;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr),
      _occupied(false),
      _trusted_memory(nullptr)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
    : _trusted_memory(trusted)
{
    _occupied_ptr = read_first_alloc(trusted);
    _occupied = (_occupied_ptr != nullptr);
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}

std::pmr::memory_resource*
allocator_boundary_tags::get_parent_allocator() const noexcept
{
    return *reinterpret_cast<std::pmr::memory_resource**>(_trusted_memory);
}

allocator_boundary_tags::fit_mode
allocator_boundary_tags::get_fitmode() const noexcept
{
    return *reinterpret_cast<fit_mode*>(
        reinterpret_cast<char*>(_trusted_memory)
        + sizeof(std::pmr::memory_resource*)
    );
}

size_t allocator_boundary_tags::read_space_size(void *trusted) noexcept
{
    return *reinterpret_cast<size_t*>(
        reinterpret_cast<char*>(trusted)
        + sizeof(std::pmr::memory_resource*)
        + sizeof(fit_mode)
    );
}

std::mutex& allocator_boundary_tags::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex*>(
        reinterpret_cast<char*>(_trusted_memory)
        + sizeof(std::pmr::memory_resource*)
        + sizeof(fit_mode)
        + sizeof(size_t)
    );
}

void* allocator_boundary_tags::read_first_alloc(void *trusted) noexcept
{
    return *reinterpret_cast<void**>(
        reinterpret_cast<char*>(trusted)
        + sizeof(std::pmr::memory_resource*)
        + sizeof(fit_mode)
        + sizeof(size_t)
        + sizeof(std::mutex)
    );
}

void* allocator_boundary_tags::read_block_back(void *block) noexcept
{
    return *reinterpret_cast<void**>(
        reinterpret_cast<char*>(block) + sizeof(size_t)
    );
}

void* allocator_boundary_tags::read_block_forward(void *block) noexcept
{
    return *reinterpret_cast<void**>(
        reinterpret_cast<char*>(block)
        + sizeof(size_t)
        + sizeof(void*) * 2
    );
}

size_t allocator_boundary_tags::read_block_size(void *block) noexcept
{
    return *reinterpret_cast<size_t*>(block);
}

void* allocator_boundary_tags::read_block_parent(void *block) noexcept
{
    return *reinterpret_cast<void**>(
        reinterpret_cast<char*>(block)
        + sizeof(size_t)
        + sizeof(void*) * 3
    );
}

size_t allocator_boundary_tags::get_space_size() const noexcept
{
    return read_space_size(_trusted_memory);
}

void* allocator_boundary_tags::get_first_free() const noexcept
{
    return read_first_alloc(_trusted_memory);
}

void allocator_boundary_tags::set_first_free(void *ptr) noexcept
{
    *reinterpret_cast<void**>(
        reinterpret_cast<char*>(_trusted_memory)
        + sizeof(std::pmr::memory_resource*)
        + sizeof(fit_mode)
        + sizeof(size_t)
        + sizeof(std::mutex)
    ) = ptr;
}