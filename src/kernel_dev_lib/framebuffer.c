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
 * framebuffer.c
 *  Created on: Jan 6, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "nyarlathotep.h"

static uint8_t fg_color[3] = {0xFF, 0xFF, 0xFF};
static uint8_t bg_color[3] = {0xFF, 0xFF, 0xFF};

int32_t framebuffer_width() {
	return dev_sys_0arg(DEV_SYS_FRAMEBUFFER_GET_WIDTH);
}

int32_t framebuffer_height() {
	return dev_sys_0arg(DEV_SYS_FRAMEBUFFER_GET_HEIGHT);
}

int framebuffer_update() {
	return dev_sys_0arg(DEV_SYS_FRAMEBUFFER_UPDATE);
}

int framebuffer_write(uint8_t* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	return dev_sys_5arg(DEV_SYS_FRAMEBUFFER_WRITE, (ruint_t)(uintptr_t)data, x, y, w, h);
}

int framebuffer_read(uint8_t* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	return DS_ERROR_NOT_IMPLEMENTED;
}

int framebuffer_clear_screen(uint8_t r, uint8_t g, uint8_t b) {
	return dev_sys_3arg(DEV_SYS_FRAMEBUFFER_CLEAR, r, g, b);
}

int	framebuffer_putpixel(uint32_t x, uint32_t y) {
	return dev_sys_3arg(DEV_SYS_FRAMEBUFFER_PUTPIXEL, (ruint_t)(uintptr_t)&fg_color, x, y);
}

void sg_set_fg_color(uint8_t r, uint8_t g, uint8_t b) {
	fg_color[0] = r;
	fg_color[1] = g;
	fg_color[2] = b;
}

void sg_get_fg_color(uint8_t* r, uint8_t* g, uint8_t* b) {
	*r = fg_color[0];
	*g = fg_color[1];
	*b = fg_color[2];
}

void sg_set_bg_color(uint8_t r, uint8_t g, uint8_t b) {
	bg_color[0] = r;
	bg_color[1] = g;
	bg_color[2] = b;
}

void sg_get_bg_color(uint8_t* r, uint8_t* g, uint8_t* b) {
	*r = bg_color[0];
	*g = bg_color[1];
	*b = bg_color[2];
}

int sg_put_pixel(uint16_t x, uint16_t y) {
	return framebuffer_putpixel(x, y);
}

int sg_draw_point(sg_point_t point) {
	return sg_put_pixel(point.x, point.y);
}

#define _sg_put_pixel(x, y) do{\
	int err;\
	if ((err=sg_put_pixel(x, y))!=0)\
		return err;\
	}while(0)


int sg_draw_line(sg_line_t line) {
	int32_t dx = line.end.x - line.start.x;
	int32_t dy = line.end.y - line.start.y;

	if (dx == 0 && dy != 0) {
		for (int32_t x=line.start.x; x<=line.end.x; x++) {
			for (int32_t y=line.start.y; y<=line.end.y; y++) {
				_sg_put_pixel(x, y);
			}
		}
		return 0;
	}

	int32_t D = 2*dy - dx;
	int32_t y = line.start.y;
	_sg_put_pixel(line.start.x, line.start.y);

	for (int32_t x=line.start.x+1; x<=line.end.x; x++) {
		_sg_put_pixel(x, y);
		D = D + (2*dy);
		if (D > 0) {
			++y;
			D = D - (2*dx);
		}
	}
	return 0;
}

#define _sg_draw_line(line) do{\
	if ((err=sg_draw_line(line))!=0)\
		goto cleanup; \
	}while(0)

int sg_draw_rect(sg_rectangle_t rect, sg_draw_mode_t mode) {
	sg_line_t l1, l2, l3, l4;
	int err = 0;

	l1.start = rect.xy;
	l1.end.x = rect.xy.x + rect.w;
	l1.end.y = rect.xy.y;

	l2.start = rect.xy;
	l2.end.x = rect.xy.x;
	l2.end.y = rect.xy.y + rect.h;

	l3.start.x = rect.xy.x + rect.w;
	l3.start.y = rect.xy.y;
	l3.end.x = rect.xy.x + rect.w;
	l3.end.y = rect.xy.y + rect.h;

	l4.start.x = rect.xy.x;
	l4.start.y = rect.xy.y + rect.h;
	l4.end.x = rect.xy.x + rect.w;
	l4.end.y = rect.xy.y + rect.h;

	uint8_t _fg_color[3] = {fg_color[0], fg_color[1], fg_color[2]};
	uint8_t _bg_color[3] = {bg_color[0], bg_color[1], bg_color[2]};

	if (mode == outline || mode == fill_fg_bg || mode == fill_fg_fg) {
		sg_set_fg_color(_fg_color[0], _fg_color[1], _fg_color[2]);
		_sg_draw_line(l1);
		_sg_draw_line(l2);
		_sg_draw_line(l3);
		_sg_draw_line(l4);
	} else if (mode == fill_bg_bg){
		sg_set_fg_color(_bg_color[0], _bg_color[1], _bg_color[2]);
		_sg_draw_line(l1);
		_sg_draw_line(l2);
		_sg_draw_line(l3);
		_sg_draw_line(l4);
	}

	uint8_t* pixels = 0;

	if (mode != outline) {
		uint32_t cw = (rect.w-1);
		uint32_t ch = (rect.h-1);
		pixels = malloc(cw * ch * 3);
		if (mode == fill_fg_fg || mode == fill_inside_fg) {
			for (uint32_t i=0; i<cw*ch*3; i+=3) {
				pixels[i] = _fg_color[0];
				pixels[i+1] = _fg_color[1];
				pixels[i+2] = _fg_color[2];
			}
		}
		if (mode == fill_fg_bg || mode == fill_bg_bg || mode == fill_inside_bg) {
			for (uint32_t i=0; i<cw*ch*3; i+=3) {
				pixels[i] = _bg_color[0];
				pixels[i+1] = _bg_color[1];
				pixels[i+2] = _bg_color[2];
			}
		}

		framebuffer_write(pixels, rect.xy.x+1, rect.xy.y+1, cw, ch);
	}

cleanup:
	if (pixels != NULL)
		free(pixels);
	sg_set_fg_color(_fg_color[0], _fg_color[1], _fg_color[2]);
	sg_set_bg_color(_bg_color[0], _bg_color[1], _bg_color[2]);
	return err;
}

int sg_draw_circle(sg_circle_t rect, sg_draw_mode_t mode) {
	// TODO:
	return DS_ERROR_MODE_UNSUPPORTED;
}
