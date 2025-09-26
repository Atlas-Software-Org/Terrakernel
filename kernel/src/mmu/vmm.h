#ifndef VMM_H
#define VMM_H 1

#include <stdint.h>
#include <stdbool.h>
#include "pmm.h"
#include <basic.h>
#include <string.h>

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

bool mmap(void* vaddr, void* paddr, uint64_t flags, size_t npages);
bool unmap(void* vaddr, size_t npages);

#endif /* VMM_H */