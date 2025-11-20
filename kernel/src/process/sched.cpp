#include "sched.hpp"
#include <mem/mem.hpp>
#include <drivers/serial/print.hpp>

namespace scheduler {

static Thread* current = nullptr;
static uint32_t global_id = 0;
static uint64_t cr3 = 0;
static uint64_t flags = 0;

static void main_task() {
    while (true) {
        printf("Main thread running (ID 0)\n\r");
        yield();
    }
}

static Thread* allocate_thread(void (*fn)(), bool is_main) {
    Thread* t = (Thread*)mem::heap::malloc(sizeof(Thread));
    t->regs = {};
    t->regs.rflags = flags;
    t->regs.rip = (uint64_t)fn;
    t->regs.cr3 = cr3;
    t->id = global_id++;
    t->terminated = false;
    t->next = nullptr;

    if (!is_main) {
        t->stack = (uint64_t*)mem::vmm::valloc(1);
        t->regs.rsp = (uint64_t)t->stack + 0x1000 - 8;
        t->regs.rbp = t->regs.rsp;
        *((uint64_t*)t->regs.rsp) = 0;
    } else {
        asm volatile("mov %%rsp, %0" : "=r"(t->regs.rsp));
        t->regs.rbp = t->regs.rsp;
    }

    return t;
}

void initialise() {
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    asm volatile("pushfq; pop %0" : "=r"(flags));

    current = allocate_thread(main_task, true);
    current->next = current;

    printf("Scheduler initialized. Main thread ID=%d\n\r", current->id);
}

Thread* create(void (*fn)()) {
    Thread* t = allocate_thread(fn, false);
    
    t->next = current->next;
    current->next = t;

    return t;
}

Thread* next_runnable() {
    if (!current) return nullptr;

    Thread* start = current;
    do {
        current = current->next;
        if (!current->terminated) return current;
    } while (current != start);

    return nullptr;
}

void yield() {
    Thread* old = current;
    Thread* next = next_runnable();
    if (!next || next == old) return;
    current = next;
    switch_thread(&old->regs, &current->regs);
}

void schedule() {
    while (true) yield();
}

}