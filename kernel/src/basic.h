#ifndef BASIC_H
#define BASIC_H 1

#include <printk/printk.h>
#include <serial/serial.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void* ptr;

void outb(uint16_t port, uint8_t byte);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t word);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t dword);
uint32_t inl(uint16_t port);
void IOWait();
void insw(uint16_t port, void* addr, int count);
void outsw(uint16_t port, const void* addr, int count);

void KiPanic(const char* __restrict string, int _halt);
void DisplaySplash(int w, int h, char* text); /* w: width of display in characters, h: height of display in characters */

#define STATUS_OK 0
#define STATUS_ERR 1
void printsts(const char* msg, int sts);

void* PA2VA(void* phys_addr);
void* VA2PA(void* virt_addr);

uint64_t PA2VAu64(uint64_t phys_addr);
uint64_t VA2PAu64(uint64_t virt_addr);

#endif /* BASIC_H */
