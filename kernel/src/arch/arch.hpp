#ifndef ARCH_HPP
#define ARCH_HPP 1

#include <cstdint>

namespace arch {
namespace x86_64 {
	namespace io {
		void outb(uint16_t port, uint8_t value);
		uint8_t inb(uint16_t port);
		void outw(uint16_t port, uint16_t value);
		uint16_t inw(uint16_t port);
		void outl(uint16_t port, uint32_t value);
		uint32_t inl(uint16_t port);

		void io_wait();
	}
namespace cpu {
	namespace gdt {
		void load_tss();
		void load_gdt();
		void initialise();
	}

	namespace idt {
		void load_idt();
		void initialise();
		void set_descriptor(uint8_t vector, uint64_t isr, uint8_t flags);
		void clear_descriptor(uint8_t vector);
	}
}
}
}

#endif /* ARCH_HPP */
