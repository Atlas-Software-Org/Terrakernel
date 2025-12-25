#include "ps2m.hpp"
#include <arch/arch.hpp>
#include <cstdio>

static uint8_t packet[4];
static uint8_t packet_size = 3;
static uint8_t packet_cur = 0;

static bool has_wheel = false;

static void process_mouse() {
    bool lmb = packet[0] & 0x01;
    bool rmb = packet[0] & 0x02;
    bool mmb = packet[0] & 0x04;

    int8_t x = (int8_t)packet[1];
    int8_t y = -(int8_t)packet[2];

    int8_t wheel = 0;
    if (has_wheel)
        wheel = (int8_t)packet[3];

    printf("MOUSE REPORT:\n");
    printf("LMB=%s MMB=%s RMB=%s\n",
           lmb ? "down" : "up",
           mmb ? "down" : "up",
           rmb ? "down" : "up");
    printf("X=%d Y=%d", x, y);
    if (has_wheel)
        printf(" W=%d", wheel);
    printf("\n");

    packet_cur = 0;
}

__attribute__((interrupt))
void ps2m_interrupt_handler(void*) {
    uint8_t data = arch::x86_64::io::inb(0x60);

    if (packet_cur == 0 && !(data & 0x08)) {
        arch::x86_64::io::outb(0xA0, 0x20);
        arch::x86_64::io::outb(0x20, 0x20);
        return;
    }

    packet[packet_cur++] = data;

    if (packet_cur == packet_size)
        process_mouse();

    arch::x86_64::io::outb(0xA0, 0x20);
    arch::x86_64::io::outb(0x20, 0x20);
}

namespace drivers::input::ps2m {

static void wait_out() {
    while (arch::x86_64::io::inb(0x64) & 0x02);
}

static void wait_in() {
    while (!(arch::x86_64::io::inb(0x64) & 0x01));
}

static void write(uint8_t v) {
    wait_out();
    arch::x86_64::io::outb(0x64, 0xD4);
    wait_out();
    arch::x86_64::io::outb(0x60, v);
}

static uint8_t read() {
    wait_in();
    return arch::x86_64::io::inb(0x60);
}

static void detect_wheel() {
    write(0xF3); read();
    write(200);  read();
    write(0xF3); read();
    write(100);  read();
    write(0xF3); read();
    write(80);   read();

    write(0xF2);
    read();
    uint8_t id = read();

    if (id == 3) {
        has_wheel = true;
        packet_size = 4;
    }
}

void initialise() {
    using namespace arch::x86_64::io;

	inb(0x60); // to ensure no data waits

    outb(0x64, 0xA8);

    wait_out();
    outb(0x64, 0x20);
    wait_in();
    uint8_t status = inb(0x60);
    status |= 0x02;
    wait_out();
    outb(0x64, 0x60);
    wait_out();
    outb(0x60, status);

    write(0xF6);
    read();
    write(0xF4);
    read();

    detect_wheel();

    arch::x86_64::cpu::idt::set_descriptor(0x2C, (uint64_t)ps2m_interrupt_handler, 0x8E);
}

}
