#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <pp_allocator.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <mutex>
#include <cmath>
#include <vector>

namespace __detail
{
    constexpr size_t nearest_greater_k_of_2(size_t size) noexcept 
    {
        if (size <= 1) return 0; 
        
        size_t power = 1; 
        int k = 0;
        while (power < size) {
            power <<= 1; 
            k++;
        }
        return k;
    }
}

class allocator_buddies_system final:
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode
{
    private:


    struct block_metadata
    {
        bool occupied : 1;
        unsigned char size : 7; //получается k от 0 до 127, то есть максимальный размер блока 2^7 - 1
    };

    void *_trusted_memory;

    static constexpr const size_t allocator_metadata_size = sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char) + sizeof(std::mutex);

    static constexpr const size_t occupied_block_metadata_size = sizeof(block_metadata) + sizeof(void*);

    static constexpr const size_t free_block_metadata_size = sizeof(block_metadata);

    static constexpr const size_t min_k = __detail::nearest_greater_k_of_2(occupied_block_metadata_size); //то есть в этой системе блоки не могут быть меньше 2^k_min байт 

public:

    explicit allocator_buddies_system(
            size_t space_size_power_of_two, //размер данных в виде 2^k байт
            std::pmr::memory_resource *parent_allocator = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

    allocator_buddies_system(
        allocator_buddies_system const &other);
    
    allocator_buddies_system &operator=(
        allocator_buddies_system const &other);
    
    allocator_buddies_system(
        allocator_buddies_system &&other) noexcept;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system &&other) noexcept;

    ~allocator_buddies_system() override;

    void swap(allocator_buddies_system& other) noexcept;

private:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t size) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;


    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

    class buddy_iterator //итерируется по всем блокам
    {
        void* _block;

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const buddy_iterator&) const noexcept;

        bool operator!=(const buddy_iterator&) const noexcept;

        buddy_iterator& operator++() & noexcept;

        buddy_iterator operator++(int n);

        size_t size() const noexcept;

        bool occupied() const noexcept;

        void* operator*() const noexcept;

        buddy_iterator();

        buddy_iterator(void* start);

        void* get_block() const { return _block; }

    };

    friend class buddy_iterator;

    buddy_iterator begin() const noexcept;

    buddy_iterator end() const noexcept;
    
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H