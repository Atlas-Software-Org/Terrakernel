#include "ps2k.hpp"
#include <config.hpp>
#include <mem/mem.hpp>
#include <arch/arch.hpp>
#include <cstdio>
#include <extra_cfgs/ps2k_layout.hpp>

bool shift;
bool caps;

char* fifo_buffer;
size_t fifo_head = 0;
size_t fifo_tail = 0;
size_t fifo_size;

static bool fifo_empty() {
    return fifo_head == fifo_tail;
}

static bool fifo_full() {
    return ((fifo_tail + 1) % fifo_size) == fifo_head;
}

static void fifo_push(char c) {
    if (!fifo_full()) {
        fifo_buffer[fifo_tail] = c;
        fifo_tail = (fifo_tail + 1) % fifo_size;
    }
}

static char fifo_pop() {
    if (fifo_empty()) return 0;
    char c = fifo_buffer[fifo_head];
    fifo_head = (fifo_head + 1) % fifo_size;
    return c;
}

void handle_key(bool make, uint8_t data) {
    if (!make) return;

    char out = translate_n[data];
    bool is_letter = (out >= 'a' && out <= 'z') || (out >= 'A' && out <= 'Z');

    if (is_letter && (shift ^ caps)) out = translate_c[data];
    else if (shift) out = translate_s[data];

#ifdef PS2K_CFG_ECHO
    putchar(out);
#endif

    fifo_push(out);
}

__attribute__((interrupt))
void ps2k_interrupt_handler(void*) {
    uint8_t data = arch::x86_64::io::inb(0x60);
    bool make = !(data & 0x80);
    data &= 0x7F;

    switch (data) {
        case 0x2A: shift = make; break;
        case 0x3A: if (make) caps = !caps; break;
        default: handle_key(make, data); break;
    }

    arch::x86_64::cpu::idt::send_eoi(1);
}

namespace drivers::input::ps2k {

void initialise() {
	arch::x86_64::io::inb(0x60); // to ensure no data waits

#ifdef PS2K_CFG_ALLOC_BUF_MALLOC
    fifo_size = PS2K_CFG_INITIAL_BUF_SIZE;
    fifo_buffer = (char*)mem::heap::malloc(fifo_size);
#else
    fifo_size = PS2K_CFG_INITIAL_BUF_SIZE;
    fifo_buffer = (char*)mem::vmm::valloc((fifo_size + 0xFFF) / 0x1000);
#endif

    arch::x86_64::cpu::idt::set_descriptor(0x21, (uint64_t)ps2k_interrupt_handler, 0x8E);
    arch::x86_64::cpu::idt::send_eoi(1);
}

int readln(size_t n, char* out) {
    size_t cursor = 0;
    while (cursor < n - 1) {
        char c = fifo_pop();
        if (!c) continue;

        if (c == '\n') {
            out[cursor++] = c;
            break;
        } else if (c == '\b') {
            if (cursor > 0) cursor--;
        } else {
            out[cursor++] = c;
        }
    }
    out[cursor] = '\0';
    return cursor;
}

int read(size_t n, char* out) {
    size_t cursor = 0;
    while (cursor < n) {
        char c = fifo_pop();
        if (!c) continue;

        if (c == '\n' || c == '\0') break;
        else if (c == '\b') {
            if (cursor > 0) cursor--;
        } else {
            out[cursor++] = c;
        }
    }
    return cursor;
}

}
