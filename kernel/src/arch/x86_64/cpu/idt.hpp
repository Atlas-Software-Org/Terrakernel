#ifndef IDT_HPP
#define IDT_HPP 1

#include <cstdint>

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) idtr_t;

typedef struct {
	uint16_t isr_offset_low;
	uint16_t gdt_selector;
	uint8_t ist;
	uint8_t flags; /* usually 0x8E */
	uint16_t isr_offset_middle;
	uint32_t isr_offset_high;
	uint32_t always_zero;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	idt_entry_t entries[512];
} __attribute__((packed)) idt_t;

extern "C" void idt_load(const idtr_t* idtr);

namespace arch::x86_64::cpu::idt {

void load_idt();
void initialise();
void set_descriptor(uint8_t vector, uint64_t isr, uint8_t flags);
void clear_descriptor(uint8_t vector);

void irq_clear_mask(uint8_t irq);
void irq_set_mask(uint8_t irq);

void send_eoi(uint8_t irq);

}

#endif /* IDT_HPP */
