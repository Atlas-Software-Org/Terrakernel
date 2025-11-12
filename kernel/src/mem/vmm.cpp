#include <mem/mem.hpp>
#include <drivers/serial/print.hpp>

uint64_t default_PML4 = 0;
uint64_t current_PML4 = 0;

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

namespace mem::vmm {

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
    default_PML4 = cr3;
    current_PML4 = default_PML4;
}

void* valloc(size_t npages) {
    void* page = mem::pmm::palloc(npages);
    return (void*)mem::vmm::pa_to_va((uint64_t)page);
}

void free(void* ptr, size_t npages) {
    uint64_t page = mem::vmm::va_to_pa((uint64_t)ptr);
    mem::pmm::free((void*)page, npages);
}

void mmap(void* paddr, void* vaddr, size_t npages, uint64_t attributes) {
    uint64_t va = (uint64_t)vaddr;
    uint64_t pa = (uint64_t)paddr;

    for (size_t i = 0; i < npages; i++, va += 0x1000, pa += 0x1000) {
        uint64_t pml4_index = (va >> 39) & 0x1FF;
        uint64_t pdpt_index = (va >> 30) & 0x1FF;
        uint64_t pd_index   = (va >> 21) & 0x1FF;
        uint64_t pt_index   = (va >> 12) & 0x1FF;

        uint64_t* pml4 = reinterpret_cast<uint64_t*>(current_PML4);
        uint64_t* pdpt;
        uint64_t* pd;
        uint64_t* pt;

        if (!(pml4[pml4_index] & PAGE_PRESENT)) {
            uint64_t new_pdpt = (uint64_t)mem::pmm::palloc(1);
            mem::memset(reinterpret_cast<void*>(mem::vmm::pa_to_va(new_pdpt)), 0, 0x1000);
            pml4[pml4_index] = new_pdpt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        pdpt = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pml4[pml4_index] & ~0xFFF));

        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
            uint64_t new_pd = (uint64_t)mem::pmm::palloc(1);
            mem::memset(reinterpret_cast<void*>(mem::vmm::pa_to_va(new_pd)), 0, 0x1000);
            pdpt[pdpt_index] = new_pd | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        pd = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pdpt[pdpt_index] & ~0xFFF));

        if (!(pd[pd_index] & PAGE_PRESENT)) {
            uint64_t new_pt = (uint64_t)mem::pmm::palloc(1);
            mem::memset(reinterpret_cast<void*>(mem::vmm::pa_to_va(new_pt)), 0, 0x1000);
            pd[pd_index] = new_pt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        pt = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pd[pd_index] & ~0xFFF));

        pt[pt_index] = pa | attributes | PAGE_PRESENT;
    }

    Log::infof("Mapped %d pages (vaddr= %p/paddr= %p)\n\r", npages, vaddr, paddr);
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

        uint64_t* pdpt = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pml4[pml4_index] & ~0xFFF));
        if (!(pdpt[pdpt_index] & PAGE_PRESENT)) continue;

        uint64_t* pd = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pdpt[pdpt_index] & ~0xFFF));
        if (!(pd[pd_index] & PAGE_PRESENT)) continue;

        uint64_t* pt = reinterpret_cast<uint64_t*>(mem::vmm::pa_to_va(pd[pd_index] & ~0xFFF));
        if (!(pt[pt_index] & PAGE_PRESENT)) continue;

        pt[pt_index] = 0;

        mem::pmm::free((void*)mem::vmm::va_to_pa(va), 1);
    }

    Log::infof("Unmapped %d pages (vaddr= %p)\n\r", npages, vaddr);
}

void switch_pagetable(uint64_t ptr) {
    asm volatile ("mov %0, %%cr3" : : "r"(ptr) : "memory");
    current_PML4 = ptr;
}

void reset_pagetable() {
    switch_pagetable(default_PML4);
    current_PML4 = default_PML4;
}

}
