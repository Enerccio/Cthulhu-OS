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
 * main.c
 *  Created on: Jan 2, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include <ny/nyarlathotep.h>

int main(void) {
	framebuffer_clear_screen(0, 0, 0);

	framebuffer_putpixel(100, 100);

	sg_set_fg_color(0xFF, 0, 0);
	sg_line_t l;
	l.start.x = 50;
	l.start.y = 50;
	l.end.x = 500;
	l.end.y = 75;
	sg_draw_line(l);

	sg_set_fg_color(0, 0xFF, 0);
	sg_set_bg_color(0, 0, 0xFF);
	sg_rectangle_t rect;
	rect.xy.x = 200;
	rect.xy.y = 150;
	rect.w = 50;
	rect.h = 125;
	sg_draw_rect(rect, outline);

	rect.xy.x = 300;
	rect.xy.y = 150;
	sg_draw_rect(rect, fill_fg_bg);

	rect.xy.x = 200;
	rect.xy.y = 350;
	sg_draw_rect(rect, fill_fg_fg);

	rect.xy.x = 300;
	rect.xy.y = 350;
	sg_draw_rect(rect, fill_bg_bg);

	rect.xy.x = 500;
	rect.xy.y = 250;
	sg_draw_rect(rect, fill_inside_fg);

	rect.xy.x = 500;
	rect.xy.y = 250;
	sg_draw_rect(rect, fill_inside_bg);

	framebuffer_update();

    while (1) ;
}
