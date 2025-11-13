#include <panic.hpp>

void panic(char* error_code) {
    printf("PANIC!\n\rError code: %s\n\r");

    asm volatile ("cli;hlt;");
}