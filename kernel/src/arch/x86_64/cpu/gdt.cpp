#include <arch/arch.hpp>
#include <arch/x86_64/cpu/gdt.hpp>

namespace arch::x86_64::cpu::gdt {

alignas(16) static gdt_t gdt;
alignas(16) static gdtr_t gdtr;
alignas(16) static tss_t tss;
static uint8_t kernel_stack[4096];

static const gdt_t default_gdt = {
    {0, 0, 0, 0, 0, 0},        // Null
    {0, 0, 0, 0x9A, 0xA0, 0},  // Kernel code
    {0, 0, 0, 0x92, 0xA0, 0},  // Kernel data
    {0, 0, 0, 0xF2, 0xA0, 0},  // User data
    {0, 0, 0, 0xFA, 0xA0, 0},  // User code
    {0, 0, 0, 0x89, 0x00, 0, 0, 0} // TSS
};

void load_gdt() {
    gdt_load(&gdtr);
}

void load_tss() {
    tss_load();
}

void initialise() {
    gdt = default_gdt;

    tss.rsp0 = (uint64_t)(kernel_stack + sizeof(kernel_stack));
    tss.ist1 = (uint64_t)(kernel_stack + sizeof(kernel_stack));
    tss.iopb_offset = sizeof(tss_t);

    uint64_t base = (uint64_t)&tss;
    uint16_t limit = sizeof(tss_t) - 1;

    gdt.tss.limit_low    = limit & 0xFFFF;
    gdt.tss.base_low     = base & 0xFFFF;
    gdt.tss.base_middle1 = (base >> 16) & 0xFF;
    gdt.tss.access       = 0x89;
    gdt.tss.granularity  = 0x00;
    gdt.tss.base_middle2 = (base >> 24) & 0xFF;
    gdt.tss.base_high    = (base >> 32) & 0xFFFFFFFF;
    gdt.tss.reserved     = 0;

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base  = (uint64_t)&gdt;

    load_gdt();
    load_tss();
}

}
