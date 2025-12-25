#ifndef PS2K_HPP
#define PS2K_HPP 1

#include <cstdint>
#include <cstddef>

namespace drivers::input::ps2k {

void initialise();

int readln(size_t n, char* out);
int read(int n, char* out);

}

#endif
