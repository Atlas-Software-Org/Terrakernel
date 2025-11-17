#include <panic.hpp>

void panic(char* error_code) {
    printf("PANIC!\n\rError code: %s\n\r", error_code);

    asm volatile ("cli;hlt;");
}

void assert(bool expected) {
    if (!expected) panic("Assertion failed...\n\r");
}

void assert_specific(bool expected, const char* info) {
    if (!expected) {
        printf("Panicking... %s\n\r", info);
        panic("Assertion failed...\n\r");
    }
}