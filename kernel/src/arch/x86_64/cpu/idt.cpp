#include <arch/arch.hpp>
#include <arch/x86_64/cpu/idt.hpp>
#include <drivers/serial/print.hpp>

bool idt_set_vectors[256] = {false};

const char* exception_names[] = {
	"#DE", "#DB", "#NMI", "#BP", "#OF", "#BR",
	"#UD", "#NM", "#DF", "N/A", "#TS", "#NP",
	"#SS", "#GP", "#PF", "N/A", "#MF", "#AC",
	"#MC", "#XM", "#VE", "#CP", "N/A", "N/A",
	"N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
	"N/A", "N/A",
};

bool is_solvable(int excvec) {
	switch (excvec) {
		case 0:
		case 3:
		case 14:
			return 1;
		default:
			return 0;
	}
	return 0;
}

extern "C" void exception_handler(
	uint64_t rip,
	uint64_t exception_vector,
	uint64_t error_code,
	uint64_t cr2,
	uint64_t cr3
) {
	Log::errf(
		"EXCEPTION OCCURED!\n\r"
		"EXCEPTION_TYPE= %s\n\r"
		"EXCEPTION_RIP= 0x%llX\n\r"
		"EXCEPTION_VECTOR= %d\n\r"
		"EXCEPTION_ERRCODE= %d\n\r"
		"EXCEPTION_CR2= 0x%llX (Faulting address (if #PF) )\n\r"
		"EXCEPTION_CR3= 0x%llX (Page table)\n\r\n\r"
		"%s(%d)\n\r",
		exception_names[exception_vector],
		rip, exception_vector, error_code,
		cr2, cr3, exception_names[exception_vector],
		error_code
	);

	if (!is_solvable(exception_vector)) asm volatile ("cli;hlt;");

	Log::infof("Exception is solvable");

	asm volatile ("cli;hlt;"); // for now just proceed to halt
}

#define PIC1		0x20
#define PIC2		0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20

#define ICW1_ICW4	0x01
#define ICW1_SINGLE	0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	0x08
#define ICW1_INIT	0x10

#define ICW4_8086	0x01
#define ICW4_AUTO	0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	0x10

#define CASCADE_IRQ 2

static void remap(int offset1, int offset2) {
    arch::x86_64::io::outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC1_DATA, offset1);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC2_DATA, offset2);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC1_DATA, 1 << CASCADE_IRQ);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC2_DATA, 2);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC1_DATA, ICW4_8086);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC2_DATA, ICW4_8086);
    arch::x86_64::io::io_wait();
    arch::x86_64::io::outb(PIC1_DATA, 0xff);
    arch::x86_64::io::outb(PIC2_DATA, 0xff);
}

namespace arch::x86_64::cpu::idt {

alignas(16) static idt_t idt;
alignas(16) static idtr_t idtr;

void load_idt() {
	idt_load(&idtr);
}

extern "C" uint64_t exception_stub_table[];

void initialise() {
	remap(0x20, 0x28);

	for (int i = 0; i < 0x1F; i++) {
		idt_set_vectors[i] = true;
		set_descriptor(i, exception_stub_table[i], 0x8E);
	}

	idtr.limit = sizeof(idt) - 1;
	idtr.base = (uint64_t)&idt;

	arch::x86_64::io::outb(0x20, 0xFF);
	arch::x86_64::io::outb(0xA0, 0xFF);

	load_idt();
}	

void set_descriptor(uint8_t vector, uint64_t isr, uint8_t flags) {
	idt_entry_t *e = &idt.entries[vector];

	e->isr_offset_low = (isr & 0xFFFF);
	e->gdt_selector = 0x08; /* Kernel Code */
	e->ist = 0;
	e->flags = flags;
	e->isr_offset_middle = (isr >> 16) & 0xFFFF;
	e->isr_offset_high = (isr >> 32) & 0xFFFFFFFF;
	e->always_zero = 0;

	uint64_t recombined = e->isr_offset_low
                    | ((uint64_t)e->isr_offset_middle << 16)
                    | ((uint64_t)e->isr_offset_high << 32);

	Log::infof(
		"Loaded ISR (0x%016llX) at: low= 0x%016llX, mid= 0x%016llX, hi= 0x%016llX, addr= 0x%016llX",
		isr,
		e->isr_offset_low,
		e->isr_offset_middle,
		e->isr_offset_high,
		recombined
	);

	if (0x20 <= vector && vector < 0x30) {
		if (0x20 <= vector && vector < 0x28) {
			uint8_t mask = arch::x86_64::io::inb(0x21);
			mask &= ~(1 << (vector - 0x20));
			arch::x86_64::io::outb(0x21, mask);
		}
		else if (0x28 <= vector && vector < 0x30) {
			uint8_t mask = arch::x86_64::io::inb(0xA1);
			mask &= ~(1 << (vector - 0x28));
			arch::x86_64::io::outb(0xA1, mask);
		}
	}

	idt_set_vectors[vector] = true;
}

void clear_descriptor(uint8_t vector) {
    set_descriptor(vector, 0, 0);
    idt_set_vectors[vector] = false;

    if (0x20 <= vector && vector < 0x30) {
        if (0x20 <= vector && vector < 0x28) {
            uint8_t mask = arch::x86_64::io::inb(0x21);
            mask |= (1 << (vector - 0x20));
            arch::x86_64::io::outb(0x21, mask);
        }
        else if (0x28 <= vector && vector < 0x30) {
            uint8_t mask = arch::x86_64::io::inb(0xA1);
            mask |= (1 << (vector - 0x28));
            arch::x86_64::io::outb(0xA1, mask);
        }
    }
}

}
