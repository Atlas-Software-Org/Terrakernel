#include <drivers/serial/serial.hpp>
#include <drivers/serial/printf.h>
#include <drivers/serial/print.hpp>
#include <arch/arch.hpp>

extern "C" void init() {
	arch::x86_64::cpu::gdt::initialise();
	Log::print_status("OK", "GDT Initialised");

	asm volatile ("sti");
	while (1) {
		asm volatile ("hlt;");
	}
	__builtin_unreachable();
}
