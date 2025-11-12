#include <drivers/timers/pit.hpp>
#include <drivers/serial/print.hpp>

struct pit_interrupt {
    void (*handler)();
    uint64_t frequency_divisor;
    bool set;
};

static constexpr int PIT_FREQUENCY = 100;
static uint64_t ticks = 0;

static pit_interrupt pit_interrupts[4] = {};
static int attached = 0;

#define CHx_DATA(ch) (0x40 + (ch))
#define CHx_MODE_CMD_REG(ch) (0x43)

inline uint64_t safe_div(uint64_t numerator, uint64_t denominator) {
    if (denominator == 0) {
        Log::err("Division by zero attempted");
        return 0;
    }
    return numerator / denominator;
}

inline uint64_t safe_modulo(uint64_t numerator, uint64_t denominator) {
    if (denominator == 0) return 0;
    return numerator % denominator;
}

#include <drivers/serial/print.hpp>
#include <stdint.h>

template <typename Func, typename... Args>
inline void safe_call(Func function, Args... args) {
    if (!function) {
        Log::err("Attempted to call null function pointer in safe_call");
        return;
    }

    asm volatile("cli");
    function(args...);
    asm volatile("sti");
}

__attribute__((interrupt))
static void pit_handler(void* frame) {
    ticks++;

    for (int i = 0; i < 4; i++) {
        if (!pit_interrupts[i].set) continue;
        if (safe_modulo(ticks, pit_interrupts[i].frequency_divisor) == 0) {
            safe_call(pit_interrupts[i].handler);
        }
    }

    arch::x86_64::io::outb(0x20, 0x20);
}

namespace driver::pit {

void initialise() {
    using namespace arch::x86_64::io;

    if (PIT_FREQUENCY == 0) {
        Log::err("PIT_FREQUENCY cannot be zero");
        return;
    }

    outb(CHx_MODE_CMD_REG(0), 0x34);

    uint16_t divisor = static_cast<uint16_t>(safe_div(1193180, PIT_FREQUENCY));
    outb(CHx_DATA(0), static_cast<uint8_t>(divisor & 0xFF));
    io_wait();
    outb(CHx_DATA(0), static_cast<uint8_t>((divisor >> 8) & 0xFF));

    arch::x86_64::cpu::idt::set_descriptor(0x20, (uint64_t)pit_handler, 0x8E);

    Log::infof("PIT Initialised (PIT_FREQUENCY=%d)", PIT_FREQUENCY);
}

void attach_periodic_interrupt(void (*handler)(), uint64_t frequency_divisor) {
    if (attached < 4) {
        pit_interrupts[attached++] = {handler, frequency_divisor, true};
    }
}

void sleep_ms(uint64_t ms) {
    uint64_t current_ticks = ticks;
    uint64_t blocking_ticks = safe_div(ms * 1193180ULL, 1000ULL);
    uint64_t final_ticks = current_ticks + blocking_ticks;

    while (ticks < final_ticks) {
    }
}

uint64_t ns_elapsed_time() {
    return safe_div(ticks * 1000000000ULL, 1193180ULL);
}

}
