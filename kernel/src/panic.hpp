#ifndef PANIC_HPP
#define PANIC_HPP 1

#include <drivers/serial/printf.h>

void panic(char* error_code);
void assert(bool expected);
void assert_specific(bool expected, const char* info);

#endif /* PANIC_HPP */