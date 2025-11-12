#include <arch/arch.hpp>

namespace arch::x86_64::io {

void outb(uint16_t port, uint8_t value) {
    asm volatile(".intel_syntax noprefix;out dx, al;.att_syntax prefix" : : "a"(value), "d"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile(".intel_syntax noprefix;in al, dx;.att_syntax prefix" : "=a"(ret) : "d"(port));
    return ret;
}

void outw(uint16_t port, uint16_t value) {
    asm volatile(".intel_syntax noprefix;out dx, ax;.att_syntax prefix" : : "a"(value), "d"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile(".intel_syntax noprefix;in ax, dx;.att_syntax prefix" : "=a"(ret) : "d"(port));
    return ret;
}

void outl(uint16_t port, uint32_t value) {
    asm volatile(".intel_syntax noprefix;out dx, eax;.att_syntax prefix" : : "a"(value), "d"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile(".intel_syntax noprefix;in eax, dx;.att_syntax prefix" : "=a"(ret) : "d"(port));
    return ret;
}

void io_wait() {
    outb(0xFFFF, 0xFF);
}

}
