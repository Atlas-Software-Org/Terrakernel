#include "flanterm.h"
#include "flanterm_backends/fb.h"
#include <limine.h>
#include "ftctx.h"

__attribute__((section(".limine_requests")))
volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

struct limine_framebuffer* fb;
struct flanterm_context *ft_ctx;

uint64_t g_scr_height, g_scr_width;

void flanterm_initialise() {
    if (fb_request.response == (void*)0 || fb_request.response->framebuffer_count < 1) {
        asm volatile ("hlt;cli;");
    }

    fb = fb_request.response->framebuffers[0];

    ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        fb->address, fb->width, fb->height, fb->pitch,
        fb->red_mask_size, fb->red_mask_shift,
        fb->green_mask_size, fb->green_mask_shift,
        fb->blue_mask_size, fb->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        5
    );

    g_scr_height = fb->height;
    g_scr_width = fb->width;
}

__attribute__((always_inline, hot))
uint32_t lerpRGB(uint32_t src, uint32_t dst, uint8_t intensity) {
    uint32_t sr = (src >> 16) & 0xFF;
    uint32_t sg = (src >>  8) & 0xFF;
    uint32_t sb =  src        & 0xFF;

    uint32_t dr = (dst >> 16) & 0xFF;
    uint32_t dg = (dst >>  8) & 0xFF;
    uint32_t db =  dst        & 0xFF;

    uint32_t r = sr + ((int32_t)(dr - sr) * intensity >> 8);
    uint32_t g = sg + ((int32_t)(dg - sg) * intensity >> 8);
    uint32_t b = sb + ((int32_t)(db - sb) * intensity >> 8);

    return (r << 16) | (g << 8) | b;
}

__attribute__((always_inline, hot))
void putpx(int x, int y, uint32_t colour) {
	if (x > g_scr_width-1 || y > g_scr_height-1 || x < 0 || y < 0) return;
	uint32_t currcolour = ((volatile uint32_t *)fb->address)[y * fb->pitch + x];
	uint32_t final = lerpRGB(currcolour, colour, (colour >> 24) & 0xFF);
	((volatile uint32_t *)fb->address)[y * fb->pitch + x] = final;
}

__attribute__((always_inline, hot))
uint32_t getpx(int x, int y) {
	if (x > g_scr_width-1 || y > g_scr_height-1 || x < 0 || y < 0) return (uint32_t)-1;
	return ((volatile uint32_t *)fb->address)[y * fb->pitch + x];
}

void* get_ftctx() {return (void*)ft_ctx;}
