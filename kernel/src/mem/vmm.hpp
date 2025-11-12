#ifndef VMM_HPP
#define VMM_HPP 1

#include <mem/mem.hpp>

#include <cstddef>

namespace mem::vmm {

uint64_t pa_to_va(uint64_t pa);
uint64_t va_to_pa(uint64_t va);
		
void initialise();
void* valloc(size_t npages);
void free(void* ptr, size_t npages);
void mmap(void* paddr, void* vaddr, size_t npages, uint64_t attributes);
void munmap(void* vaddr, size_t npages);

}

#endif /* VMM_HPP */
