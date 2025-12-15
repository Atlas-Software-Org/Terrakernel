#include "sched.hpp"
#include <mem/mem.hpp>
#include <cstdio>
#include <cstring>

extern "C" void switch_context_x64(uint64_t* new_rsp, uint64_t* old_rsp);

static Task* running_task;
static Task* idle_task;
static Task* last_task;

uint64_t rflags = 0x202;

void idle() {
    for (;;) asm("hlt");
}

static bool initialised = false;

namespace sched {

void initialise() {
    if (initialised) return;
    asm("cli");
    idle_task = create_task(idle, "idle");
    running_task = idle_task;
    asm("sti");
    initialised = true;
}

Task* create_task(void (*entry)(), char* name) {
    Task* task = (Task*)mem::heap::malloc(sizeof(Task));
    if (!task) return nullptr;

    mem::memset(task, 0, sizeof(Task));

    uint64_t* stack = (uint64_t*)mem::vmm::valloc(1);
    if (!stack) {
        mem::heap::free(task);
        return nullptr;
    }

    uint64_t* sp = (uint64_t*)((void*)stack + 4096);

    sp = (uint64_t*)((uintptr_t)sp & ~0xF);

    *--sp = (uint64_t)entry;
    *--sp = 0x202;

    for (int i = 0; i < 15; i++)
        *--sp = 0;

    task->rsp = (uint64_t)sp;
    task->name = strdup(name ? name : "unnamed");
    task->next = nullptr;

    if (last_task)
        last_task->next = task;
    else
        idle_task->next = task;

    last_task = task;
    return task;
}

void yield() {
    Task* prev = running_task;
    running_task = running_task->next ? running_task->next : idle_task;
    printf("Switching from task %s to task %s\n", prev->name, running_task->name);
    switch_context_x64(&prev->rsp, &running_task->rsp);
}

void task1_func() {
    for (;;) {
        printf("Task 1 running\n");
        for (volatile int i = 0; i < 1000000; i++);
        sched::yield();
    }
}

void task2_func() {
    for (;;) {
        printf("Task 2 running\n");
        for (volatile int i = 0; i < 1000000; i++);
        sched::yield();
    }
}

void task3_func() {
    for (;;) {
        printf("Task 3 running\n");
        for (volatile int i = 0; i < 1000000; i++);
        sched::yield();
    }
}

void omar__sched_test() {
    Task* task1 = create_task(task1_func, "task1");
    Task* task2 = create_task(task2_func, "task2");
    Task* task3 = create_task(task3_func, "task3");
    sched::yield();
}

}