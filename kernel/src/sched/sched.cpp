#include "sched.hpp"
#include <mem/mem.hpp>

static Task* runningTask;
static Task* mainTask;
static Task* idle_task;

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

Task* createTask(void (*entry)()) {
    Task* task = (Task*) mem::heap::malloc(sizeof(Task));
    if (!task) {
        return nullptr;
    }

    mem::memset(task, 0, sizeof(Task));

    task->regs.rip = (uint64_t)entry;
    task->regs.rsp = (uint64_t)mem::heap::malloc(0x4000) + 0x4000;
    task->regs.cr3 = (uint64_t)mem::vmm::create_pagetable();
    task->regs.rflags = 0x202;

    task->next = idle_task->next;

    return task;
}

void yield() {
    Task *last = runningTask;
    runningTask = runningTask->next;
    switchTask(&last->regs, &runningTask->regs);
}

}