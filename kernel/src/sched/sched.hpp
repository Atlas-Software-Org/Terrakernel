#ifndef SCHED_HPP
#define SCHED_HPP 1

#include <cstdint>


struct Registers {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, rflags, cr3;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
};

struct Task {
    Registers regs;
    Task *next;
};

namespace sched {

void initialise();
Task* createTask(void (*entry)());

void yield();

}

#endif
