#include "gdt.h"
#include <stddef.h>
#include <stdint.h>

#define GDT_ENTRIES 7

static uint64_t gdt[GDT_ENTRIES];
static struct GdtPtr gdt_ptr;
struct TssEntry tss;

extern void gdt_load(struct GdtPtr* gdt_ptr, uint16_t tss_selector);

static void gdt_set_gate(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[idx] = ((uint64_t)limit & 0xFFFF) |
               (((uint64_t)base & 0xFFFFFF) << 16) |
               ((uint64_t)access << 40) |
               (((uint64_t)gran & 0xF0) << 48) |
               (((uint64_t)limit & 0xF0000) << 48) |
               (((uint64_t)base & 0xFF000000) << 32);
}

static void gdt_set_tss(int idx, uint64_t base, uint32_t limit) {
    // TSS descriptor spans two entries
    gdt[idx]     = ((uint64_t)(limit & 0xFFFF)) |
                   ((base & 0xFFFFFF) << 16) |
                   ((uint64_t)0x89 << 40) |
                   ((uint64_t)((limit >> 16) & 0x0F) << 48) |
                   ((base & 0xFF000000) << 32);

    gdt[idx + 1] = base >> 32;
}

void gdt_init() {
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);       // Null
    gdt_set_gate(1, 0, 0, 0x9A, 0x20); // Kernel code
    gdt_set_gate(2, 0, 0, 0x92, 0x00); // Kernel data
    gdt_set_gate(3, 0, 0, 0xFA, 0x20); // User code
    gdt_set_gate(4, 0, 0, 0xF2, 0x00); // User data

    gdt_set_tss(5, (uint64_t)&tss, sizeof(tss)-1); // TSS occupies entries 5 and 6

    gdt_load(&gdt_ptr, 0x28); // TSS selector = 5 << 3
}
