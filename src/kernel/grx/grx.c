/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * grx.c
 *  Created on: Dec 29, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "grx.h"
#include "font.h"

#include "../utils/kstdlib.h"
#include "../memory/paging.h"
#include "../rlyeh/rlyeh.h"
#include <ds/array.h>

#define FB_COLOR_MODE_EGA  0
#define FB_COLOR_MODE_IDXC 1
#define FB_COLOR_MODE_RGB  2

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

uint8_t mode;
bool __print_initialized;

color_t ega[16] = {
    {0, 0, 0}, {0, 0, 0xAA}, {0, 0xAA, 0}, {0, 0xAA, 0xAA}, {0xAA, 0, 0}, {0xAA, 0, 0xAA},
    {0xAA, 0x55, 0}, {0xAA, 0xAA, 0xAA}, {0x55, 0x55, 0x55}, {0x55, 0x55, 0xFF},
    {0x55, 0xFF, 0x55}, {0x55, 0xFF, 0xFF},
    {0xFF, 0x55, 0x55}, {0xFF, 0x55, 0xFF}, {0xFF, 0xFF, 0x55}, {0xFF, 0xFF, 0xFF}
};
image_t* clear_screen_blit;

static uint8_t* framebuffer;
static uint32_t* local_fb;
static uint8_t* local_fb_changes;
static uint32_t w, h, fbpitch, bpp;
static color_t gray;
static uint8_t rmask, bmask, gmask;
static uint8_t rpos, bpos, gpos;

extern uint16_t* text_mode_video_memory;
extern void kp_halt();

void set_graphics_mode(struct multiboot_info* mb) {
    mode = MODE_TEXT;

    if ((mb->flags & (1<<11)) == 0) {
        w = 80;
        h = 24;
        return;
    }

    if ((mb->flags & (1<12)) == 0) {
        w = 80;
        h = 24;
        return;
    }

    framebuffer = (uint8_t*)physical_to_virtual(mb->framebuffer_addr);
    w = mb->framebuffer_width;
    h = mb->framebuffer_height;
    fbpitch = mb->framebuffer_pitch;
    bpp = mb->framebuffer_bpp;

    if (mb->framebuffer_type == 2) {
        // EGA text mode, standard MODE_TEXT
        text_mode_video_memory = (uint16_t*)framebuffer;
        return;
    }

    if (mb->framebuffer_type == 0) {
        log_err("Framebuffer unsupported mode INDEXED");
        kp_halt();
    }

    mode = MODE_GRAPHICS;
    rmask = mb->framebuffer_red_mask_size;
    gmask = mb->framebuffer_green_mask_size;
    bmask = mb->framebuffer_blue_mask_size;
    rpos = mb->framebuffer_red_field_position;
    gpos = mb->framebuffer_green_field_position;
    bpos = mb->framebuffer_blue_field_position;

    local_fb = malloc(w*h*32);
    memset(local_fb, 0, w*h*32);

    local_fb_changes = malloc(BITNSLOTS(w*h));
    memset(local_fb_changes, 0, BITNSLOTS(w*h));

    gray.b = gray.r = gray.g = 127;
    initialize_font();

    clear_screen_blit = malloc(sizeof(image_t));
    clear_screen_blit->image_type = IMAGE_MONOCHROMATIC;
    clear_screen_blit->image_data = malloc(((fbpitch*h)/8)+1);
    memset(clear_screen_blit->image_data, 0xFF, ((fbpitch*h)/8)+1);
    clear_screen_blit->h = grx_get_height();
    clear_screen_blit->w = grx_get_width();
}

void initialize_grx(struct multiboot_info* mb) {
    set_graphics_mode(mb);
    __print_initialized = true;
}

uint32_t grx_get_height() {
    return h;
}

uint32_t grx_get_width() {
    return w;
}

void blit(image_t* image, uint32_t x, uint32_t y) {
    blit_colored(image, x, y, gray);
}

void blit_colored(image_t* image, uint32_t x, uint32_t y, color_t recolor) {
    uint32_t iw = image->w;
    uint32_t ih = image->h;

    for (uint32_t py = y; py < y+ih; py++) {
        if (py >= h) continue;
        for (uint32_t px = x; px < x+iw; px++) {
            if (px >= w) continue;

            size_t pixelpos = ((py*w)+px);

            bool drawn = true;
            if (image->image_type == IMAGE_RGB) {
                size_t bps = ((py-y)*image->w)+(px-x)*3;
                recolor.r = image->image_data[bps];
                recolor.g = image->image_data[bps+1];
                recolor.b = image->image_data[bps+2];
            } else if (image->image_type == IMAGE_MONOCHROMATIC) {
                size_t bsp = ((py-y)*image->w)+(px-x);
                drawn = BITTEST(image->image_data, bsp);
            }

            if (drawn) {
                uint32_t color = recolor.b << 16 | recolor.g << 8 | recolor.r;
                local_fb[pixelpos] = color;
                BITSET(local_fb_changes, pixelpos);
            }
        }
    }

}

void scroll_up(uint32_t bypx) {
    memmove(local_fb, local_fb+(bypx*w), (h-bypx)*w*4);
    memset(local_fb+((h-bypx)*w), 0, bypx*w*4);
    memset(local_fb_changes, 0xFF, BITNSLOTS(w*h));
    flush_buffer();
}

void flush_buffer() {
    size_t ppx = 0;
    for (size_t i=0; i<w*h; i++) {
        if (BITTEST(local_fb_changes, i)) {
            BITCLEAR(local_fb_changes, i);
            uint32_t color = local_fb[i];
            if (bpp == 32) {
                memcpy(&local_fb_changes[i], &framebuffer[ppx], 4);
            } else if (bpp == 24) {
                uint8_t red =  (color & 0xFF);
                uint8_t green =  ((color>>8) & 0xFF);
                uint8_t blue =  ((color>>16) & 0xFF);
                framebuffer[ppx] = blue;
                framebuffer[ppx+1] = green;
                framebuffer[ppx+2] = red;
            }
        }
        if (bpp == 32)
            ppx += 4;
        if (bpp == 24)
            ppx += 3;
    }
}
