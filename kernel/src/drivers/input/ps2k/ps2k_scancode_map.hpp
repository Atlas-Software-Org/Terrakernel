#ifndef PS2K_SCANCODE_MAP_HPP
#define PS2K_SCANCODE_MAP_HPP 1
#include "ps2k_keycodes.hpp"

keycode scancode_to_keycode[128];
keycode extended_scancode_to_keycode[128];

inline void init_scancode_map() {
    for (int i = 0; i < 128; ++i) {
        scancode_to_keycode[i] = KEY_NONE;
        extended_scancode_to_keycode[i] = KEY_NONE;
    }
    
    scancode_to_keycode[0x1E] = KEY_A;
    scancode_to_keycode[0x30] = KEY_B;
    scancode_to_keycode[0x2E] = KEY_C;
    scancode_to_keycode[0x20] = KEY_D;
    scancode_to_keycode[0x12] = KEY_E;
    scancode_to_keycode[0x21] = KEY_F;
    scancode_to_keycode[0x22] = KEY_G;
    scancode_to_keycode[0x23] = KEY_H;
    scancode_to_keycode[0x17] = KEY_I;
    scancode_to_keycode[0x24] = KEY_J;
    scancode_to_keycode[0x25] = KEY_K;
    scancode_to_keycode[0x26] = KEY_L;
    scancode_to_keycode[0x32] = KEY_M;
    scancode_to_keycode[0x31] = KEY_N;
    scancode_to_keycode[0x18] = KEY_O;
    scancode_to_keycode[0x19] = KEY_P;
    scancode_to_keycode[0x10] = KEY_Q;
    scancode_to_keycode[0x13] = KEY_R;
    scancode_to_keycode[0x1F] = KEY_S;
    scancode_to_keycode[0x14] = KEY_T;
    scancode_to_keycode[0x16] = KEY_U;
    scancode_to_keycode[0x2F] = KEY_V;
    scancode_to_keycode[0x11] = KEY_W;
    scancode_to_keycode[0x2D] = KEY_X;
    scancode_to_keycode[0x15] = KEY_Y;
    scancode_to_keycode[0x2C] = KEY_Z;
    
    scancode_to_keycode[0x0B] = KEY_0;
    scancode_to_keycode[0x02] = KEY_1;
    scancode_to_keycode[0x03] = KEY_2;
    scancode_to_keycode[0x04] = KEY_3;
    scancode_to_keycode[0x05] = KEY_4;
    scancode_to_keycode[0x06] = KEY_5;
    scancode_to_keycode[0x07] = KEY_6;
    scancode_to_keycode[0x08] = KEY_7;
    scancode_to_keycode[0x09] = KEY_8;
    scancode_to_keycode[0x0A] = KEY_9;
    
    scancode_to_keycode[0x3B] = KEY_F1;
    scancode_to_keycode[0x3C] = KEY_F2;
    scancode_to_keycode[0x3D] = KEY_F3;
    scancode_to_keycode[0x3E] = KEY_F4;
    scancode_to_keycode[0x3F] = KEY_F5;
    scancode_to_keycode[0x40] = KEY_F6;
    scancode_to_keycode[0x41] = KEY_F7;
    scancode_to_keycode[0x42] = KEY_F8;
    scancode_to_keycode[0x43] = KEY_F9;
    scancode_to_keycode[0x44] = KEY_F10;
    scancode_to_keycode[0x57] = KEY_F11;
    scancode_to_keycode[0x58] = KEY_F12;
    
    scancode_to_keycode[0x01] = KEY_ESC;
    scancode_to_keycode[0x0E] = KEY_BACKSPACE;
    scancode_to_keycode[0x0F] = KEY_TAB;
    scancode_to_keycode[0x1C] = KEY_ENTER;
    scancode_to_keycode[0x39] = KEY_SPACE;
    
    scancode_to_keycode[0x1D] = KEY_LEFTCTRL;
    scancode_to_keycode[0x2A] = KEY_LEFTSHIFT;
    scancode_to_keycode[0x36] = KEY_RIGHTSHIFT;
    scancode_to_keycode[0x38] = KEY_LEFTALT;
    scancode_to_keycode[0x3A] = KEY_CAPSLOCK;
    scancode_to_keycode[0x45] = KEY_NUMLOCK;
    scancode_to_keycode[0x46] = KEY_SCROLLLOCK;
    
    scancode_to_keycode[0x0C] = KEY_MINUS;
    scancode_to_keycode[0x0D] = KEY_EQUAL;
    scancode_to_keycode[0x1A] = KEY_LEFTBRACE;
    scancode_to_keycode[0x1B] = KEY_RIGHTBRACE;
    scancode_to_keycode[0x27] = KEY_SEMICOLON;
    scancode_to_keycode[0x28] = KEY_APOSTROPHE;
    scancode_to_keycode[0x29] = KEY_GRAVE;
    scancode_to_keycode[0x2B] = KEY_BACKSLASH;
    scancode_to_keycode[0x33] = KEY_COMMA;
    scancode_to_keycode[0x34] = KEY_DOT;
    scancode_to_keycode[0x35] = KEY_SLASH;
    
    scancode_to_keycode[0x47] = KEY_KP7;
    scancode_to_keycode[0x48] = KEY_KP8;
    scancode_to_keycode[0x49] = KEY_KP9;
    scancode_to_keycode[0x4A] = KEY_KPMINUS;
    scancode_to_keycode[0x4B] = KEY_KP4;
    scancode_to_keycode[0x4C] = KEY_KP5;
    scancode_to_keycode[0x4D] = KEY_KP6;
    scancode_to_keycode[0x4E] = KEY_KPPLUS;
    scancode_to_keycode[0x4F] = KEY_KP1;
    scancode_to_keycode[0x50] = KEY_KP2;
    scancode_to_keycode[0x51] = KEY_KP3;
    scancode_to_keycode[0x52] = KEY_KP0;
    scancode_to_keycode[0x53] = KEY_KPDOT;
    
    extended_scancode_to_keycode[0x48] = KEY_UP;
    extended_scancode_to_keycode[0x50] = KEY_DOWN;
    extended_scancode_to_keycode[0x4B] = KEY_LEFT;
    extended_scancode_to_keycode[0x4D] = KEY_RIGHT;
    
    extended_scancode_to_keycode[0x47] = KEY_HOME;
    extended_scancode_to_keycode[0x4F] = KEY_END;
    extended_scancode_to_keycode[0x49] = KEY_PAGEUP;
    extended_scancode_to_keycode[0x51] = KEY_PAGEDOWN;
    extended_scancode_to_keycode[0x52] = KEY_INSERT;
    extended_scancode_to_keycode[0x53] = KEY_DELETE;
    
    extended_scancode_to_keycode[0x1D] = KEY_RIGHTCTRL;
    extended_scancode_to_keycode[0x38] = KEY_RIGHTALT;
    extended_scancode_to_keycode[0x5B] = KEY_LEFTMETA;   // Action key
    extended_scancode_to_keycode[0x5C] = KEY_RIGHTMETA;  // Action key
    extended_scancode_to_keycode[0x5D] = KEY_COMPOSE;
    
    extended_scancode_to_keycode[0x35] = KEY_KPSLASH;
    extended_scancode_to_keycode[0x1C] = KEY_KPENTER;
    
    extended_scancode_to_keycode[0x22] = KEY_MUTE;
    extended_scancode_to_keycode[0x24] = KEY_VOLUMEDOWN;
    extended_scancode_to_keycode[0x2E] = KEY_VOLUMEUP;
    extended_scancode_to_keycode[0x20] = KEY_MEDIA_MUTE;
    extended_scancode_to_keycode[0x30] = KEY_MEDIA_VOLUMEUP;
    extended_scancode_to_keycode[0x2E] = KEY_MEDIA_VOLUMEDOWN;
    extended_scancode_to_keycode[0x19] = KEY_MEDIA_NEXT;
    extended_scancode_to_keycode[0x10] = KEY_MEDIA_PREV;
    extended_scancode_to_keycode[0x24] = KEY_MEDIA_STOP;
    extended_scancode_to_keycode[0x22] = KEY_MEDIA_PLAYPAUSE;
    
    extended_scancode_to_keycode[0x5E] = KEY_POWER;
    extended_scancode_to_keycode[0x5F] = KEY_SLEEP;
    extended_scancode_to_keycode[0x63] = KEY_WAKEUP;
}

#endif
