#ifndef PANIC_HPP
#define PANIC_HPP 1

#include <drivers/serial/printf.h>

void panic(char* error_code);

#endif /* PANIC_HPP */