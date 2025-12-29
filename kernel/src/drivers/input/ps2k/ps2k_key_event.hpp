#ifndef PS2K_KEY_EVENT_HPP
#define PS2K_KEY_EVENT_HPP
#include <cstdint>

enum class key_state : uint8_t {
    RELEASED = 0,
    PRESSED  = 1
};

struct key_event {
    uint16_t keycode;
    key_state state;
    uint64_t timestamp;
};

#endif
