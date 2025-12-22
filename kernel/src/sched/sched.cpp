#include "sched.hpp"
#include <mem/mem.hpp>

static Task* runningTask;
static Task* mainTask;
static Task* idle_task;

static Task ttable[256];
uint64_t free_bitmap[4] = {0};

Task* get_free_task_entry() {
    for (int i = 0; i < 256; i++) {
        int bitmap_index = i / 64;
        int bit_index = i % 64;

        if (!(free_bitmap[bitmap_index] & (1ULL << bit_index))) {
            free_bitmap[bitmap_index] |= (1ULL << bit_index);
            return &ttable[i];
        }
    }
    return nullptr;
}

void release_task_entry(Task* task) {
    uintptr_t index = task - ttable;
    if (index >= 256) return;

    int bitmap_index = index / 64;
    int bit_index = index % 64;

    free_bitmap[bitmap_index] &= ~(1ULL << bit_index);
}

void idle() {
    for (;;) {
        sched::yield();
    }
}

namespace sched {

extern "C" void switchTask(Registers* last, Registers* next);

void initialise() {
    mainTask = (Task*) mem::heap::malloc(sizeof(Task));

    asm volatile("movq %%cr3, %%rax; movq %%rax, %0;" : "=m"(mainTask->regs.cr3)::"%rax");
    asm volatile("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;" : "=m"(mainTask->regs.rflags) : : "%rax");

    idle_task = createTask(idle);

    runningTask = mainTask;
}

#include <config.hpp>

Task* createTask(void (*entry)()) {
    Task* task = get_free_task_entry();
    if (!task) {
        return nullptr;
    }

    mem::memset(task, 0, sizeof(Task));

    task->regs.rip = (uint64_t)entry;
    task->regs.rsp = (uint64_t)mem::heap::malloc(0x4000) + 0x4000;
#ifndef SCHED_CFG_SAME_PML4
#warning "Using same PML4 for tasks"
    task->regs.cr3 = 0;
#else
#warning "Using separate PML4 for tasks"
    task->regs.cr3 = (uint64_t)mem::vmm::create_pagetable();
#endif
    task->regs.rflags = 0x202;

    if (idle_task) {
        task->next = idle_task->next;
    } else {
        task->next = task;
        idle_task = task;
    }

    return task;
}

void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;
    switchTask(&last->regs, &runningTask->regs);
}

}