#include "ps2k.hpp"
#include <mem/mem.hpp>
#include <panic.hpp>
#include <arch/arch.hpp>
#include <config.hpp>

namespace driver::input::ps2k {

static char* ps2k_buf;
static size_t ps2k_buf_sz;
static volatile size_t ps2k_head;
static volatile size_t ps2k_tail;
static volatile bool locked = false;
bool ps2k_cfg_canonical = true;

void disable_canonical() { ps2k_cfg_canonical = false; }
void enable_canonical() { ps2k_cfg_canonical = true; }

static void lock() { while (locked) { asm("pause"); } locked = true; }
static void unlock() { locked = false; }

bool modifiers[9] = { false };
bool extended = false;

#define LEFT_CTL    0
#define LEFT_SHFT   1
#define RIGHT_SHFT  2
#define LEFT_ALT    3
#define CAPS_LOCK   4
#define NUM_LOCK    5
#define SCROLL_LOCK 6
#define RIGHT_CTL   7
#define RIGHT_ALT   8

char nrml_translate_scancode[128] =
    "\x00123456789-=\bqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0789-456+230.\0\0\0\0\0";

char shft_translate_scancode[128] =
    "\x00!@#$%^&*()_+\bQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0789-456+230.\0\0\0\0\0";

char caps_translate_scancode[128] =
    "\x00123456789-=\bQWERTYUIOP[]\n\0ASDFGHJKL;'`\0\\ZXCVBNM,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0789-456+230.\0\0\0\0\0";

static bool ps2k_push_char(char c) {
    lock();
    size_t next = (ps2k_head + 1) % ps2k_buf_sz;
    if (next == ps2k_tail) { unlock(); return false; }
    ps2k_buf[ps2k_head] = c;
    ps2k_head = next;
    unlock();
    return true;
}

static bool ps2k_pop_char(char &out) {
    lock();
    if (ps2k_tail == ps2k_head) { unlock(); return false; }
    out = ps2k_buf[ps2k_tail];
    ps2k_tail = (ps2k_tail + 1) % ps2k_buf_sz;
    unlock();
    return true;
}

void ps2k_flush() { lock(); ps2k_head = ps2k_tail = 0; unlock(); }

__attribute__((interrupt))
void ps2k_interrupt_handler(void*);

void initialise() {
    lock();
#if PS2K_CFG_INITIAL_BUF_SIZE < 1
#error "PS2K_CFG_INITIAL_BUF_SIZE must be >= 1"
#endif
#ifdef PS2K_CFG_ALLOC_BUF_MALLOC
    ps2k_buf = (char*)mem::heap::malloc(PS2K_CFG_INITIAL_BUF_SIZE);
#else
    ps2k_buf = (char*)mem::vmm::valloc((PS2K_CFG_INITIAL_BUF_SIZE + 0xFFF)/0x1000);
#endif
    if (!ps2k_buf) panic("ps2k_buf allocation failed");
    ps2k_buf_sz = PS2K_CFG_INITIAL_BUF_SIZE;
    ps2k_head = ps2k_tail = 0;
    arch::x86_64::cpu::idt::set_descriptor(0x21, (uint64_t)ps2k_interrupt_handler, 0x8E);
    unlock();
}

static void handle_normal(uint8_t key) {
    char c = 0;
    if (modifiers[LEFT_SHFT] || modifiers[RIGHT_SHFT]) c = shft_translate_scancode[key];
    else if (modifiers[CAPS_LOCK]) c = caps_translate_scancode[key];
    else c = nrml_translate_scancode[key];
    if (c != 0) ps2k_push_char(c);
    if (ps2k_cfg_canonical && c != 0) putchar(c);
}

static void handle_extended(uint8_t key) {}

__attribute__((interrupt))
void ps2k_interrupt_handler(void*) {
    if (!(arch::x86_64::io::inb(0x64) & 1)) { arch::x86_64::io::outb(0x20, 0x20); return; }
    uint8_t data = arch::x86_64::io::inb(0x60);
    bool released = data & 0x80;
    uint8_t key = data & 0x7F;
    if (data == 0xE0) { extended = true; goto end; }
    if (!extended) {
        switch (key) {
            case 0x1D: modifiers[LEFT_CTL] = !released; break;
            case 0x2A: modifiers[LEFT_SHFT] = !released; break;
            case 0x36: modifiers[RIGHT_SHFT] = !released; break;
            case 0x38: modifiers[LEFT_ALT] = !released; break;
            case 0x3A: if (!released) modifiers[CAPS_LOCK] = !modifiers[CAPS_LOCK]; break;
            case 0x45: if (!released) modifiers[NUM_LOCK] = !modifiers[NUM_LOCK]; break;
            case 0x46: if (!released) modifiers[SCROLL_LOCK] = !modifiers[SCROLL_LOCK]; break;
            default: if (!released) handle_normal(key); break;
        }
    } else {
        switch (key) {
            case 0x1D: modifiers[RIGHT_CTL] = !released; break;
            case 0x38: modifiers[RIGHT_ALT] = !released; break;
            default: if (!released) handle_extended(key); break;
        }
        extended = false;
    }
end:
    arch::x86_64::cpu::idt::send_eoi(1);
}

size_t ps2k_read_raw(size_t max_buf, void* buf, bool) {
    size_t read = 0; char c;
    while (read < max_buf) { while (!ps2k_pop_char(c)); ((char*)buf)[read++] = c; }
    return read;
}

size_t ps2k_read_canonical(size_t max_buf, void* buf, bool) {
    size_t read = 0; char c;
    while (read < max_buf) { while (!ps2k_pop_char(c)); if (c == '\n') break; ((char*)buf)[read++] = c; }
    return read;
}

}
