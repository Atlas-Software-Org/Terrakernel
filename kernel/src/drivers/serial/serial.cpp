#include "serial.hpp"

#include <arch/arch.hpp>

namespace serial {
	extern "C" void serial_putc(char c) {
		arch::x86_64::io::outb(0x3F8, c);
	}
}
