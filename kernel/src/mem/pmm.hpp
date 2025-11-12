#ifndef PMM_HPP
#define PMM_HPP 1

#include <mem/mem.hpp>

#include <cstdint>

#define STAT_TOTAL_MEM			0
#define STAT_USED_MEM			1
#define STAT_FREE_MEM			2
#define STAT_ALLOC_COUNT		3
#define STAT_FAILED_ALLOC_COUNT	4
#define STAT_FREE_COUNT			5
#define STAT_FAILED_FREE_COUNT	6

namespace mem::pmm {

uint64_t stat_free();
uint64_t stat_used();
uint64_t stat_total_mem();

void stat_print();

uint64_t stat_get_status(uint8_t type); 

void initialise();
void* palloc(size_t npages);
void free(void* ptr, size_t npages);

void* reserve_heap(size_t npages);

}

#endif /* PMM_HPP */
