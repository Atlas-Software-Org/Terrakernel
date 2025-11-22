#include "sched.hpp"
#include <drivers/serial/print.hpp>
#include <mem/mem.hpp>

Context _kernel_context;
Context *_context = &_kernel_context;
uint64_t _rip = 0;

Process* current_proc = nullptr;
Process* root = nullptr;

pid_t pids = 0;
bool ready = false;

#define DBG_SCHED

#ifdef DBG_SCHED
#define sched_dbgf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define sched_dbgf(fmt, ...)
#endif

Process* get_prev(Process* p) {
    sched_dbgf("[get_prev] looking for previous of %p\n\r", p);
    if (!p || !root || p == root) {
        sched_dbgf("[get_prev] no previous found (nullptr/root)\n\r");
        return nullptr;
    }

    Process* last = nullptr;
    Process* cur = root;
    while (cur && cur != p) {
        last = cur;
        cur = cur->next;
    }

    sched_dbgf("[get_prev] returning %p\n\r", (cur == p) ? last : nullptr);
    return (cur == p) ? last : nullptr;
}

void connect_prev_with_next(Process* p) {
    sched_dbgf("[connect_prev_with_next] removing %p\n\r", p);
    if (!p || !root) { sched_dbgf("[connect_prev_with_next] nothing to do\n\r"); return; }

    if (p == root) {
        root = root->next;
        sched_dbgf("[connect_prev_with_next] root removed, new root %p\n\r", root);
        mem::heap::free(p);
        return;
    }

    Process* prev = get_prev(p);
    if (prev) {
        sched_dbgf("[connect_prev_with_next] linking %p->next = %p\n\r", prev, p->next);
        prev->next = p->next;
    } else {
        sched_dbgf("[connect_prev_with_next] no prev found for %p\n\r", p);
    }

    mem::heap::free(p);
}

Process* get_proc_with_pid(pid_t pid) {
    sched_dbgf("[get_proc_with_pid] looking for PID=%d\n\r", pid);
    Process* p = root;
    while (p) {
        if (p->pid == pid) {
            sched_dbgf("[get_proc_with_pid] found PID=%d at %p\n\r", pid, p);
            return p;
        }
        p = p->next;
    }
    sched_dbgf("[get_proc_with_pid] PID=%d not found\n\r", pid);
    return nullptr;
}

pid_t get_next_pid() {
    static pid_t next_pid = 0;

    pid_t start = next_pid;

    do {
        if (next_pid == (pid_t)-1) next_pid = 0;

        if (!get_proc_with_pid(next_pid)) {
            pid_t assigned = next_pid;
            next_pid++;
            sched_dbgf("[get_next_pid] assigned PID=%d\n\r", assigned);
            return assigned;
        }

        next_pid++;
    } while (next_pid != start);

    sched_dbgf("[get_next_pid] no available PID found\n\r");
    return -1;
}

Process* get_next_proc() {
    if (!root) { sched_dbgf("[get_next_proc] root is nullptr\n\r"); return nullptr; }

    Process* p = root;
    while (p->next)
        p = p->next;

    sched_dbgf("[get_next_proc] last process = %p\n\r", p);
    return p;
}

Process* get_next_proc_alloc() {
    Process* p = (Process*)mem::heap::malloc(sizeof(Process));
    sched_dbgf("[get_next_proc_alloc] allocated %p\n\r", p);
    mem::memset(p, 0, sizeof(Process));
    p->next = nullptr;

    if (!root) {
        root = p;
        sched_dbgf("[get_next_proc_alloc] root set to %p\n\r", root);
        return p;
    }

    Process* last = get_next_proc();
    last->next = p;
    sched_dbgf("[get_next_proc_alloc] linked %p->next = %p\n\r", last, p);
    return p;
}

void zeroproc() {
    for (;;) {
        sched_dbgf("[zeroproc] running PID=%d current_proc=%p\n\r", scheduler::current_pid(), current_proc);
        asm("hlt");
    }
}

namespace scheduler {

pid_t current_pid() {
    return current_proc != nullptr ? current_proc->pid : -1 ;
}

void initialise() {
    sched_dbgf("[scheduler::initialise] spawning zeroproc\n\r");
    pid_t zproc_pid = spawn(zeroproc, true, true, nullptr, nullptr);
    current_proc = get_proc_with_pid(zproc_pid);
    sched_dbgf("[scheduler::initialise] current_proc = %p PID=%d\n\r", current_proc, current_pid());
}

pid_t spawn(void (*entry)(), bool kstack, bool kheap, void* stack_base, void* heap_base) {
    sched_dbgf("[spawn] entry=%p\n\r", entry);
    asm("cli");

    Process* p = get_next_proc_alloc();
    p->pid = get_next_pid();
    if (p->pid == -1) { asm("sti"); sched_dbgf("[spawn] failed to get PID\n\r"); return -1; }

    sched_dbgf("[spawn] PID=%d allocated at %p\n\r", p->pid, p);
    p->stack_base = stack_base ? stack_base : mem::vmm::valloc(4096);
    p->heap_base = heap_base ? heap_base : 0;

    mem::memset(&p->ctx, 0, sizeof(Context));
    p->ctx.rip = (uint64_t)entry;
    p->ctx.rbp = (uint64_t)p->stack_base + 0x1000;
    p->ctx.rsp = (uint64_t)p->stack_base + 0x1000;

    p->next = root;
    root = p;
    p->ready = true;

    sched_dbgf("[spawn] inserted at head, root=%p\n\r", root);

    if (!current_proc) {
        current_proc = p;
        sched_dbgf("[spawn] set current_proc=%p\n\r", current_proc);
    }

    asm("sti");
    return p->pid;
}

void exit(pid_t pid) {
    sched_dbgf("[exit] PID=%d\n\r", pid);
    asm("cli");

    Process* p = get_proc_with_pid(pid);
    if (!p) { asm("sti"); sched_dbgf("[exit] PID not found\n\r"); return; }

    p->ready = false;
    connect_prev_with_next(p);

    if (current_proc == p) {
        current_proc = p->next ? p->next : root;
        sched_dbgf("[exit] current_proc switched to %p\n\r", current_proc);
    }

    asm("sti");
}

Process* next_proc_yield() {
    sched_dbgf("[next_proc_yield] current_proc=%p\n\r", current_proc);
    if (!current_proc) return nullptr;
    Process* next = current_proc->next ? current_proc->next : root;
    sched_dbgf("[next_proc_yield] next=%p\n\r", next);
    return next;
}

void yield(uint64_t rip) {
    if (!ready) return;
    
    sched_dbgf("[yield] called with RIP=%llx current_proc=%p\n\r", rip, current_proc);
    asm("cli");

    if (!ready) { sched_dbgf("[yield] scheduler not ready\n\r"); asm("sti"); return; }

    Process* this_ = current_proc;
    Process* next_ = next_proc_yield();
    if (!next_) { sched_dbgf("[yield] no next process\n\r"); asm("sti"); return; }

    if (this_ && _context) {
        sched_dbgf("[yield] storing context of %p\n\r", this_);
        store_context();
        mem::memcpy(&this_->ctx, _context, sizeof(Context));
    }

    sched_dbgf("[yield] loading context of %p\n\r", next_);
    mem::memcpy(_context, &next_->ctx, sizeof(Context));
    current_proc = next_;

    asm("sti");
    load_context();
}

void begin() {
    sched_dbgf("[scheduler::begin] scheduler ready\n\r");
    ready = true;
}

bool is_ready() {
    return ready;
}

}