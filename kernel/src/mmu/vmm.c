#include "vmm.h"
#include "pmm.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_SIZE 0x1000
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PAGE_ATTR_MASK 0xFFFUL

typedef struct PageTable {
    uint64_t Entries[512];
} PageTable;

static inline PageTable* get_or_create_table(uint64_t entry, bool* created) {
    if (!(entry & PAGE_PRESENT)) {
        void* page = palloc(1);
        if (!page) return NULL;
        memset(page, 0, PAGE_SIZE);
        if (created) *created = true;
        return (PageTable*)PA2VAu64(page);
    }
    return (PageTable*)PA2VAu64(entry & PAGE_ADDR_MASK);
}

bool mmap(void* vaddr, void* paddr, uint64_t flags, size_t npages) {
    if (!vaddr || !paddr || npages == 0) return false;

    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    PageTable* pml4 = (PageTable*)PA2VAu64(cr3 & PAGE_ADDR_MASK);

    for (size_t page = 0; page < npages; page++) {
        uint64_t va = (uint64_t)vaddr + page * PAGE_SIZE;
        uint64_t pa = (uint64_t)paddr + page * PAGE_SIZE;

        uint16_t pml4_index = (va >> 39) & 0x1FF;
        uint16_t pdpt_index = (va >> 30) & 0x1FF;
        uint16_t pd_index   = (va >> 21) & 0x1FF;
        uint16_t pt_index   = (va >> 12) & 0x1FF;

        bool created = false;
        PageTable* pdpt = get_or_create_table(pml4->Entries[pml4_index], &created);
        if (!pdpt) return false;
        if (created) pml4->Entries[pml4_index] = VA2PAu64(pdpt) | PAGE_PRESENT | PAGE_RW;

        created = false;
        PageTable* pd = get_or_create_table(pdpt->Entries[pdpt_index], &created);
        if (!pd) return false;
        if (created) pdpt->Entries[pdpt_index] = VA2PAu64(pd) | PAGE_PRESENT | PAGE_RW;

        created = false;
        PageTable* pt = get_or_create_table(pd->Entries[pd_index], &created);
        if (!pt) return false;
        if (created) pd->Entries[pd_index] = VA2PAu64(pt) | PAGE_PRESENT | PAGE_RW;

        pt->Entries[pt_index] = (pa & PAGE_ADDR_MASK) | (flags & PAGE_ATTR_MASK) | PAGE_PRESENT;

        asm volatile ("invlpg (%0)" :: "r"(va) : "memory");
    }
    return true;
}

bool unmap(void* vaddr, size_t npages) {
    if (!vaddr || npages == 0) return false;

    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    PageTable* pml4 = (PageTable*)PA2VAu64(cr3 & PAGE_ADDR_MASK);

    for (size_t page = 0; page < npages; page++) {
        uint64_t va = (uint64_t)vaddr + page * PAGE_SIZE;

        uint16_t pml4_index = (va >> 39) & 0x1FF;
        uint16_t pdpt_index = (va >> 30) & 0x1FF;
        uint16_t pd_index   = (va >> 21) & 0x1FF;
        uint16_t pt_index   = (va >> 12) & 0x1FF;

        if (!(pml4->Entries[pml4_index] & PAGE_PRESENT)) continue;
        PageTable* pdpt = (PageTable*)PA2VAu64(pml4->Entries[pml4_index] & PAGE_ADDR_MASK);
        if (!(pdpt->Entries[pdpt_index] & PAGE_PRESENT)) continue;
        PageTable* pd = (PageTable*)PA2VAu64(pdpt->Entries[pdpt_index] & PAGE_ADDR_MASK);
        if (!(pd->Entries[pd_index] & PAGE_PRESENT)) continue;
        PageTable* pt = (PageTable*)PA2VAu64(pd->Entries[pd_index] & PAGE_ADDR_MASK);

        pt->Entries[pt_index] &= ~PAGE_PRESENT;
        asm volatile ("invlpg (%0)" :: "r"(va) : "memory");
    }
    return true;
}
