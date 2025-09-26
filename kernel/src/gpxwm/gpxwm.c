#include "gpxwm.h"

WMManager g_WM_ins;
WMManager* g_WM;

uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << g_WM->red_shift) |
           ((uint32_t)g << g_WM->green_shift) |
           ((uint32_t)b << g_WM->blue_shift);
}

void PutPx(int x, int y, uint32_t color) {
    if (x < 0 || x >= g_WM->dpy.xlen || y < 0 || y >= g_WM->dpy.ylen) return;
    uint32_t *fb = (uint32_t *)g_WM->dpy.fbbase;
    fb[y * (g_WM->dpy.pitch / 4) + x] = color;
}

void Flip() {
    volatile uint32_t* temp = g_WM->dpy.fbbase1;
    g_WM->dpy.fbbase1 = g_WM->dpy.fbbase2;
    g_WM->dpy.fbbase2 = temp;
    g_WM->dpy.fbbase  = g_WM->dpy.fbbase1;
}

void WMInit(struct limine_framebuffer* fb) {
    g_WM = &g_WM_ins;

    g_WM->dpy.xlen  = fb->width;
    g_WM->dpy.ylen  = fb->height;
    g_WM->dpy.pitch = fb->pitch;
    g_WM->dpy.bpp   = fb->bpp;
    g_WM->dpy.fblen = fb->width * fb->height * (fb->bpp / 8);

    g_WM->dpy.fbbase1 = (volatile uint32_t*)fb->address;
    g_WM->dpy.fbbase2 = g_WM->dpy.fbbase1; 
    g_WM->dpy.fbbase  = g_WM->dpy.fbbase1;

    g_WM->count = 0;
    for (int i = 0; i < MAX_WINDOWS; i++)
        g_WM->windows[i] = NULL;

    g_WM->red_shift   = fb->red_mask_shift;
    g_WM->green_shift = fb->green_mask_shift;
    g_WM->blue_shift  = fb->blue_mask_shift;
}

void WMPutPxWindow(WMWindow *win, int x, int y, uint32_t color) {
    if (!win) return;
    if (x < 0 || x >= win->rect.width || y < 0 || y >= win->rect.height) return;

    uint32_t *fb = (uint32_t *)g_WM->dpy.fbbase;
    int fb_pitch = g_WM->dpy.pitch / 4;

    int abs_x = win->rect.x + x;
    int abs_y = win->rect.y + y;

    if (abs_x < 0 || abs_x >= g_WM->dpy.xlen || abs_y < 0 || abs_y >= g_WM->dpy.ylen) return;
    fb[abs_y * fb_pitch + abs_x] = color;
}

WMWindow* WMCreateWindow(const char *title, uint32_t color) {
    if (g_WM->count >= MAX_WINDOWS) return NULL;

    static WMWindow windows[MAX_WINDOWS];
    WMWindow *win = &windows[g_WM->count];

    win->title = title;
    win->color = color;

    int cols = 3;
    int tile_w = g_WM->dpy.xlen / cols - TILE_MARGIN;
    int tile_h = g_WM->dpy.ylen / ((g_WM->count / cols) + 1) - TILE_MARGIN;

    int idx = g_WM->count;
    win->tile_x = idx % cols;
    win->tile_y = idx / cols;

    win->rect.x = win->tile_x * (tile_w + TILE_MARGIN) + TILE_MARGIN;
    win->rect.y = win->tile_y * (tile_h + TILE_MARGIN) + TILE_MARGIN;
    win->rect.width = tile_w;
    win->rect.height = tile_h;

    g_WM->windows[g_WM->count++] = win;
    return win;
}

void WMDrawAll() {
    // background grid
    for (int y = 0; y < g_WM->dpy.ylen; y++) {
        for (int x = 0; x < g_WM->dpy.xlen; x++) {
            PutPx(x, y, (x % 8 == 0 || y % 8 == 0) ? RGB(0,255,0) : RGB(0,0,0));
        }
    }

    // windows
    for (int i = 0; i < g_WM->count; i++) {
        WMWindow *w = g_WM->windows[i];
        if (!w) continue;

        // title bar
        for (int y = 0; y < TITLE_HEIGHT; y++)
            for (int x = 0; x < w->rect.width; x++)
                WMPutPxWindow(w, x, y, RGB(0,255,0));

        // window body
        for (int y = TITLE_HEIGHT; y < w->rect.height; y++)
            for (int x = 0; x < w->rect.width; x++)
                WMPutPxWindow(w, x, y, w->color);
    }

    Flip();
}
