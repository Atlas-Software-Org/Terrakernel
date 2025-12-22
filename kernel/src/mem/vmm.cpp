#include <mem/mem.hpp>
#include <cstdio>

uint64_t original_PML4 = 0;
uint64_t default_PML4 = 0;
uint64_t current_PML4 = 0;

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

namespace mem::vmm {

void* create_pagetable() {
    void* page = mem::pmm::palloc(1);
    if (!page) return nullptr;

    mmap(page, page, 1, PAGE_PRESENT | PAGE_RW);

    mem::memset(page, 0, 0x1000);

    constexpr int HHDM_START = 256;
    constexpr int PML4_ENTRIES = 512;

    uint64_t* new_pml4 = reinterpret_cast<uint64_t*>(page);
    uint64_t* orig_pml4 = reinterpret_cast<uint64_t*>(original_PML4);

    for (int i = HHDM_START; i < PML4_ENTRIES; i++) {
        new_pml4[i] = orig_pml4[i];
    }

    return page;
}

void free_pagetable(void* pml4_ptr) {
    if (current_PML4 == (uint64_t)pml4_ptr) {
        reset_pagetable();
    }

    mem::pmm::free(pml4_ptr, 1);
}

uint64_t fetch_default_pagetable() {
    return default_PML4;
}

uint64_t pa_to_va(uint64_t pa) {
    if (pa > 0xFFFF800000000000) return pa;
    return pa + 0xFFFF800000000000;
}

uint64_t va_to_pa(uint64_t va) {
    if (va < 0xFFFF800000000000) return va;
    return va - 0xFFFF800000000000;
}

void initialise() {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    
    original_PML4 = cr3;
    default_PML4 = cr3;
    current_PML4 = cr3;
}

void print_mem() {}

void* valloc(size_t npages) {
    void* page = mem::pmm::palloc(npages);
    return (void*)mem::vmm::pa_to_va((uint64_t)page);
}

void free(void* ptr, size_t npages) {
    uint64_t page = mem::vmm::va_to_pa((uint64_t)ptr);
    mem::pmm::free((void*)page, npages);
}

bool is_mapped(void* vaddr) {
    uint64_t va = (uint64_t)vaddr;

    uint64_t pml4_index = (va >> 39) & 0x1FF;
    uint64_t pdpt_index = (va >> 30) & 0x1FF;
    uint64_t pd_index   = (va >> 21) & 0x1FF;
    uint64_t pt_index   = (va >> 12) & 0x1FF;

    uint64_t* pml4 = reinterpret_cast<uint64_t*>(current_PML4);
    if (!(pml4[pml4_index] & PAGE_PRESENT)) return false;

    uint64_t* pdpt = reinterpret_cast<uint64_t*>(pa_to_va(pml4[pml4_index] & ~0xFFF));
    if (!(pdpt[pdpt_index] & PAGE_PRESENT)) return false;

    uint64_t* pd = reinterpret_cast<uint64_t*>(pa_to_va(pdpt[pdpt_index] & ~0xFFF));
    if (!(pd[pd_index] & PAGE_PRESENT)) return false;

    uint64_t* pt = reinterpret_cast<uint64_t*>(pa_to_va(pd[pd_index] & ~0xFFF));
    if (!(pt[pt_index] & PAGE_PRESENT)) return false;

    return true;
}

uint64_t mmap(void* paddr, void* vaddr, size_t npages, uint64_t attributes) {
    printf("mmap(%p, %p, %llu, %llu);\n\r", paddr, vaddr, npages, attributes);
    if (is_mapped(vaddr)) munmap(vaddr, npages);

    uint64_t va = (uint64_t)vaddr;
    uint64_t pa = (uint64_t)paddr;

    uint64_t first_page;

    for (size_t i = 0; i < npages; i++, va += 0x1000, pa += 0x1000) {
        uint64_t pml4_index = (va >> 39) & 0x1FF;
        uint64_t pdpt_index = (va >> 30) & 0x1FF;
        uint64_t pd_index   = (va >> 21) & 0x1FF;
        uint64_t pt_index   = (va >> 12) & 0x1FF;

        if (current_PML4 == 0) Log::warnf("PML4 is zero");

        uint64_t* pml4 = (uint64_t*)current_PML4;
        uint64_t* pdpt;
        uint64_t* pd;
        uint64_t* pt;

        uint64_t intermediate_flags = PAGE_PRESENT | PAGE_RW;

        if (!(pml4[pml4_index] & PAGE_PRESENT)) {
            uint64_t* new_pdpt = (uint64_t*)valloc(1);
            mem::memset(new_pdpt, 0, 0x1000);
            pml4[pml4_index] = va_to_pa((uint64_t)new_pdpt) | intermediate_flags;
        }
        pdpt = reinterpret_cast<uint64_t*>(pa_to_va(pml4[pml4_index] & ~0xFFF));

        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t* new_pd = (uint64_t*)valloc(1);
            mem::memset(new_pd, 0, 0x1000);
            pdpt[pdpt_index] = va_to_pa((uint64_t)new_pd) | intermediate_flags;
        }
        pd = reinterpret_cast<uint64_t*>(pa_to_va(pdpt[pdpt_index] & ~0xFFF));

        if (!(pd[pd_index] & PAGE_PRESENT)) {
            uint64_t* new_pt = (uint64_t*)valloc(1);
            mem::memset(new_pt, 0, 0x1000);
            pd[pd_index] = va_to_pa((uint64_t)new_pt) | intermediate_flags;
        }
        pt = reinterpret_cast<uint64_t*>(pa_to_va(pd[pd_index] & ~0xFFF));

        uint64_t leaf_flags = PAGE_PRESENT | (attributes & (PAGE_RW | PAGE_USER));
        pt[pt_index] = (pa & ~0xFFF) | leaf_flags;

        first_page = pt[pt_index];

        Log::infof("Page table entry [0x%016llX]: 0x%016llX", va, pt[pt_index]);
    }

    return first_page;
}

void munmap(void* vaddr, size_t npages) {
    uint64_t va = (uint64_t)vaddr;

    for (size_t i = 0; i < npages; i++, va += 0x1000) {
        uint64_t pml4_index = (va >> 39) & 0x1FF;
        uint64_t pdpt_index = (va >> 30) & 0x1FF;
        uint64_t pd_index   = (va >> 21) & 0x1FF;
        uint64_t pt_index   = (va >> 12) & 0x1FF;

        uint64_t* pml4 = reinterpret_cast<uint64_t*>(current_PML4);
        if (!(pml4[pml4_index] & PAGE_PRESENT)) continue;

        uint64_t* pdpt = reinterpret_cast<uint64_t*>(pa_to_va(pml4[pml4_index] & ~0xFFF));
        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) continue;

        uint64_t* pd = reinterpret_cast<uint64_t*>(pa_to_va(pdpt[pdpt_index] & ~0xFFF));
        if (!(pd[pd_index] & PAGE_PRESENT)) continue;

        uint64_t* pt = reinterpret_cast<uint64_t*>(pa_to_va(pd[pd_index] & ~0xFFF));
        if (!(pt[pt_index] & PAGE_PRESENT)) continue;

        pt[pt_index] = 0;
    }
}

void switch_pagetable(uint64_t ptr) {
    asm volatile ("mov %0, %%cr3" : : "r"(va_to_pa(ptr)) : "memory");
    current_PML4 = ptr;
}

void reset_pagetable() {
    switch_pagetable(default_PML4);
    current_PML4 = default_PML4;
}

}
