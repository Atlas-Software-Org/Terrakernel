#ifndef GPXWM_H
#define GPXWM_H 1

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define MAX_WINDOWS 6
#define TILE_MARGIN 4
#define TITLE_HEIGHT 24

typedef struct {
    int x, y;
    int width, height;
} Rect;

typedef struct {
    const char *title;
    uint32_t color;
    Rect rect;
    int tile_x, tile_y;
} WMWindow;

typedef struct {
    struct {
        int xlen, ylen;
        int pitch;
        int bpp;
        size_t fblen;
        volatile uint32_t *fbbase;
        volatile uint32_t *fbbase1;
        volatile uint32_t *fbbase2;
    } dpy;

    WMWindow *windows[MAX_WINDOWS];
    int count;

    int red_shift;
    int green_shift;
    int blue_shift;
} WMManager;

extern WMManager g_WM_ins;
extern WMManager *g_WM;

uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
void PutPx(int x, int y, uint32_t color);
void Flip(void);

void WMInit(struct limine_framebuffer *fb);
void WMPutPxWindow(WMWindow *win, int x, int y, uint32_t color);
WMWindow *WMCreateWindow(const char *title, uint32_t color);
void WMDrawAll(void);

#endif /* GPXWM_H */
