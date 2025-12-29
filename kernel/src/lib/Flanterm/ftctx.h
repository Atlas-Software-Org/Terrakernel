#ifndef FTCTX_H
#define FTCTX_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void flanterm_initialise();

__attribute__((always_inline, hot))
void putpx(int x, int y, uint32_t colour);
__attribute__((always_inline, hot))
uint32_t getpx(int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* FTCTX_H */
