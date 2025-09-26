#ifndef PMM_H
#define PMM_H 1

#include <basic.h>
#include <string.h>
#include <stdint.h>

int pmm_init(uint64_t largest1sz, uint64_t largest1base, uint64_t largest2sz, uint64_t largest2base, uint64_t largest3sz, uint64_t largest3base, uint64_t largest4sz, uint64_t largest4base);

void pmm_setup_page_tables();
void* palloc(uint64_t npages);
void pfree(void* ptr);

#endif /* PMM_H */
