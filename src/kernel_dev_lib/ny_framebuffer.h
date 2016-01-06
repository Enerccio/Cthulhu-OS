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
 * ny_framebuffer.h
 *  Created on: Jan 6, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "ny_stddef.h"

int32_t framebuffer_width();
int32_t framebuffer_height();
int     framebuffer_update();
int     framebuffer_write(uint8_t* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
int     framebuffer_read(uint8_t* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
int		framebuffer_clear_screen(uint8_t r, uint8_t g, uint8_t b);
int		framebuffer_putpixel(uint32_t x, uint32_t y);

typedef enum sg_draw_mode {
	outline, fill_fg_fg, fill_fg_bg, fill_bg_bg, fill_inside_fg, fill_inside_bg
} sg_draw_mode_t;

typedef struct sg_point {
	uint16_t x, y;
} sg_point_t;

typedef struct sg_line {
	sg_point_t start, end;
} sg_line_t;

typedef struct sg_rectangle {
	sg_point_t xy;
	uint16_t w, h;
} sg_rectangle_t;

typedef struct sg_circle {
	sg_point_t xy;
	uint16_t radius;
} sg_circle_t;

// sg = simple graphics
void sg_set_fg_color(uint8_t r, uint8_t g, uint8_t b);
void sg_set_bg_color(uint8_t r, uint8_t g, uint8_t b);
void sg_get_fg_color(uint8_t* r, uint8_t* g, uint8_t* b);
void sg_get_bg_color(uint8_t* r, uint8_t* g, uint8_t* b);
int sg_put_pixel(uint16_t x, uint16_t y);
int sg_draw_point(sg_point_t point);
int sg_draw_line(sg_line_t line);
int sg_draw_rect(sg_rectangle_t rect, sg_draw_mode_t mode);
int sg_draw_circle(sg_circle_t rect, sg_draw_mode_t mode);
