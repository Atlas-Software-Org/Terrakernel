#ifndef SCHED_HPP
#define SCHED_HPP

#include <cstdint>

struct Task {
    uint64_t rsp;
    Task* next;
    char* name;
};

namespace sched {
void initialise();
Task* create_task(void (*func)(), char* name = nullptr);
void yield();

void omar__sched_test();

}

#endif
