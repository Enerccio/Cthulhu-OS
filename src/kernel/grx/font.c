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
 * font.c
 *  Created on: Dec 29, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "font.h"

#include "../utils/rsod.h"
#include "../utils/kstdlib.h"

#include "font_source.h"

static image_t __letters[256];

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void initialize_font(struct multiboot_info* mbinfo) {
	for (size_t i=0; i<255*__font_h; i++)
		__font[i] = reverse(__font[i]);

	for (uint8_t i=0; i<255; i++) {
		image_t* image = &__letters[i];
		image->image_data = &__font[i*__font_h];
		image->image_type = IMAGE_MONOCHROMATIC;
		image->w = __font_w;
		image->h = __font_h;
	}
}

image_t* get_letter(char letter) {
	return &__letters[(uint8_t)letter];
}
