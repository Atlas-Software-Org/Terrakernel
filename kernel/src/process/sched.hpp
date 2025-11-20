#ifndef SCHED_HPP
#define SCHED_HPP 1

#include <stdint.h>

typedef struct {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, rflags, cr3;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
} Registers;

struct Thread {
    Registers regs;
    uint64_t stack_base;
    Thread *next;
    uint32_t pid;
    bool TERMINATED = false;
};

namespace scheduler {

void initialise();
Thread* create_thread(void (*entry)(), uint64_t rflags, uint64_t* pml4 = 0);
Thread* spawn_thread(void (*entry)());

void yield();
extern "C" void switch_thread(Registers *old_regs, Registers *new_regs);

}

#endif