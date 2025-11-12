#include <drivers/serial/serial.hpp>
#include <drivers/serial/printf.h>
#include <drivers/serial/print.hpp>
#include <arch/arch.hpp>
#include <mem/mem.hpp>

#include <drivers/timers/pit.hpp>

extern "C" void init() {
    arch::x86_64::cpu::gdt::initialise();
    Log::print_status("OK", "GDT Initialised");

    arch::x86_64::cpu::idt::initialise();
    Log::print_status("OK", "IDT Initialised");

    mem::pmm::initialise();
    Log::print_status("OK", "PMM Initialised");

    mem::vmm::initialise();
    Log::print_status("OK", "VMM Initialised");

    mem::heap::initialise();
    Log::print_status("OK", "Heap Initialised");

    driver::pit::initialise();
    Log::print_status("OK", "PIT Initialised");

    asm volatile("sti");

    while (1) {
        asm volatile("hlt;");
    }

    __builtin_unreachable();
}
