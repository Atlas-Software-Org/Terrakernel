#include "ps2m.hpp"
#include <arch/arch.hpp>
#include <cstdio>
#include <lib/Flanterm/gfx.h>

extern int g_scr_width, g_scr_height;

#define PS2Leftbutton 0b00000001
#define PS2Middlebutton 0b00000100
#define PS2Rightbutton 0b00000010
#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000

void mouse_wait() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if ((arch::x86_64::io::inb(0x64) & 0b10) == 0) {
            return;
        }
    }
}

void mouse_waitinput() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if (arch::x86_64::io::inb(0x64) & 0b1) {
            return;
        }
    }
}

void mouse_write(uint8_t value) {
    mouse_wait();
    arch::x86_64::io::outb(0x64, 0xD4);
    mouse_wait();
    arch::x86_64::io::outb(0x60, value);
}

uint8_t mouse_read() {
    mouse_waitinput();
    return arch::x86_64::io::inb(0x60);
}

uint8_t mouse_cycle = 0;
uint8_t mouse_packet[4];
bool mouse_packet_ready = false;
struct {
    int x, y;
} mouse_pos, mouse_pos_old;

void process_mouse();

__attribute__((interrupt, hot))
void ps2m_interrupt_handler(void*) {
    uint8_t data = arch::x86_64::io::inb(0x60);

    process_mouse();
    static bool skip = true;
    if (skip) { skip = false; goto end; }

    switch(mouse_cycle) {
        case 0:
           
            if ((data & 0b00001000) == 0) break;
            mouse_packet[0] = data;
            mouse_cycle++;
            break;
        case 1:
           
            mouse_packet[1] = data;
            mouse_cycle++;
            break;
        case 2:
            
            mouse_packet[2] = data;
            mouse_packet_ready = true;
            mouse_cycle = 0;
            break;
    }

end:

    arch::x86_64::cpu::idt::send_eoi(12);
}

void process_mouse() {
    if (!mouse_packet_ready) return;

    bool xNegative, yNegative, xOverflow, yOverflow;

    if (mouse_packet[0] & PS2XSign) {
        xNegative = true;
    } else xNegative = false;

    if (mouse_packet[0] & PS2YSign) {
        yNegative = true;
    } else yNegative = false;

    if (mouse_packet[0] & PS2XOverflow) {
        xOverflow = true;
    } else xOverflow = false;

    if (mouse_packet[0] & PS2YOverflow) {
        yOverflow = true;
    } else yOverflow = false;

    if (!xNegative) {
        mouse_pos.x += mouse_packet[1];
        if (xOverflow) {
            mouse_pos.x += 255;
        }
    } else 
    {
        mouse_packet[1] = 256 - mouse_packet[1];
        mouse_pos.x -= mouse_packet[1];
        if (xOverflow) {
            mouse_pos.x -= 255;
        }
    }

    if (!yNegative) {
        mouse_pos.y -= mouse_packet[2];
        if (yOverflow) {
            mouse_pos.y -= 255;
        }
    } else {
        mouse_packet[2] = 256 - mouse_packet[2];
        mouse_pos.y += mouse_packet[2];
        if (yOverflow) {
            mouse_pos.y += 255;
        }
    }

    if (mouse_pos.x < 0) mouse_pos.x = 0;
    if (mouse_pos.x > g_scr_width) mouse_pos.x = g_scr_width;
    
    if (mouse_pos.y < 0) mouse_pos.y = 0;
    if (mouse_pos.y > g_scr_height) mouse_pos.y = g_scr_height;
    
    draw_mouse_pointer(mouse_pos_old.x, mouse_pos_old.y, mouse_pos.x, mouse_pos.y, mouse_packet[0] & PS2Leftbutton, mouse_packet[0] & PS2Middlebutton, mouse_packet[0] & PS2Rightbutton);

    mouse_packet_ready = false;
    mouse_pos_old = mouse_pos;
}
            
namespace drivers::input::ps2m {

void initialise() {
    arch::x86_64::io::outb(0x64, 0xA8);

    mouse_wait();
    arch::x86_64::io::outb(0x64, 0x20);
    mouse_waitinput();
    uint8_t status = arch::x86_64::io::inb(0x60);
    status |= 0b10;
    mouse_wait();
    arch::x86_64::io::outb(0x64, 0x60);
    mouse_wait();
    arch::x86_64::io::outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();

    arch::x86_64::cpu::idt::set_descriptor(0x2C, (uint64_t)ps2m_interrupt_handler, 0x8E);
    arch::x86_64::cpu::idt::irq_clear_mask(2);
}

}