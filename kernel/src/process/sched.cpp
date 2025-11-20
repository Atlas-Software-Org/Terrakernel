#include "sched.hpp"
#include <mem/mem.hpp>
#include <drivers/serial/print.hpp>

static Thread* main_thread = nullptr;
static Thread* running_thread = nullptr;
static Thread* last_created_thread = nullptr;

static uint64_t cr3, rflags;

static Thread* __create_thread(bool is_main, uint32_t pid, void (*entry)(), uint64_t rflags, uint64_t* pml4) {
    Thread* thread = (Thread*)mem::heap::malloc(sizeof(Thread));
    if (!thread) return nullptr;

    uint64_t _cr3 = (pml4 && *pml4 != 0) ? *pml4 : cr3;

    thread->regs = {};
    thread->regs.rflags = rflags;
    thread->regs.rip = (uint64_t)entry;
    thread->regs.cr3 = _cr3;
    thread->pid = pid;
    thread->TERMINATED = false;
    thread->next = nullptr;

    if (is_main) {
        thread->stack_base = 0;
        thread->next = thread;
        last_created_thread = thread;
    } else {
        thread->stack_base = (uint64_t)mem::vmm::valloc(1);
        thread->regs.rsp = thread->stack_base + 0x1000 - 8;
        thread->regs.rbp = thread->regs.rsp;
        *((uint64_t*)thread->regs.rsp) = 0;

        last_created_thread->next = thread;
        thread->next = main_thread;
        last_created_thread = thread;
    }

    return thread;
}

namespace scheduler {

uint32_t next_pid = 1;

static void zeroproc();

void initialise() {
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    asm volatile("pushfq; pop %0" : "=r"(rflags));

    main_thread = __create_thread(true, 0, zeroproc, rflags, nullptr);
    running_thread = main_thread;

    printf("Main thread initialized. PID=%d\n\r", main_thread->pid);
}

Thread* create_thread(void (*entry)(), uint64_t rflags, uint64_t* pml4) {
    uint32_t pid = next_pid++;
    return __create_thread(false, pid, entry, rflags, pml4);
}

Thread* spawn_thread(void (*entry)()) {
    return create_thread(entry, 0x202, nullptr);
}

void yield() {
    Thread* last = running_thread;
    running_thread = running_thread->next;

    while (running_thread->TERMINATED)
        running_thread = running_thread->next;

    printf("Switching to PID %d\n\r", running_thread->pid);
    switch_thread(&last->regs, &running_thread->regs);
}

void exit_thread(Thread* thread) {
    if (!thread || thread == main_thread) return;

    thread->TERMINATED = true;

    Thread* prev = main_thread;
    while (prev->next != thread)
        prev = prev->next;

    prev->next = thread->next;

    mem::heap::free((void*)thread->stack_base);
    mem::heap::free(thread);

    if (thread == running_thread)
        yield();
}

static void zeroproc() {
    while (true) {
        printf("Main thread running (PID 0)\n\r");
        yield();
    }
}

}
