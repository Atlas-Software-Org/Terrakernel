#ifndef PS2K_HPP
#define PS2K_HPP 1

#include <cstdint>
#include <cstddef>

namespace driver::input::ps2k {

void initialise();
size_t ps2k_read_canonical(size_t max_buf, void* buf, bool ring_buf);
size_t ps2k_read_raw(size_t max_buf, void* buf, bool ring_buf);
void ps2k_flush();

void disable_canonical();
void enable_canonical();

// default
#define ps2k_read ps2k_read_canonical

}

#endif
