#ifndef SCHED_HPP
#define SCHED_HPP 1

#include <cstdint>

typedef int32_t pid_t;

struct Context {
    uint64_t rip;
    uint64_t cr3;
    uint64_t rsi, rdi;
    uint64_t r15, r14, r13, r12,
             r11, r10, r9, r8,
             rdx, rcx, rbx, rax;
    uint64_t rbp, rsp;
};

struct Process {
    pid_t pid;
    
    void* stack_base;
    void* heap_base;

    Context ctx;

    Process* next;

    bool ready;
};

namespace scheduler {

pid_t current_pid();

void initialise();

pid_t spawn(void (*entry)(), bool kstack = true, bool kheap = true, void* stack_base = nullptr, void* heap_base = nullptr);
void exit(pid_t pid);
pid_t current_pid();

void yield(uint64_t rip);

void begin();

bool is_ready();

extern "C" void store_context();
extern "C" void load_context();

}

#endif