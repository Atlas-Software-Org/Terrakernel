#include "serial.hpp"

#include <arch/arch.hpp>

namespace serial {
	bool serial_enabled = true;

	extern "C" void serial_putc(char c) {
		if (!serial_enabled) return;
		arch::x86_64::io::outb(0x3F8, c);
	}

	void serial_enable() {
		serial_enabled = true;
	}

	void serial_disable() {
		serial_enabled = false;
	}
}
