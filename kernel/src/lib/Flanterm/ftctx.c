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
        0
    );
}

void* get_ftctx() {return (void*)ft_ctx;}