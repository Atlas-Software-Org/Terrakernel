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

void draw_mouse_pointer(int old_x, int old_y, int x, int y, int button_state_lmb, int button_state_mmb, int button_state_rmb);
void fb_clrscr(int no_cur);
void refresh();

#ifdef __cplusplus
}
#endif

#endif /* FTCTX_H */
