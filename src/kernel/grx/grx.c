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
#include "image.h"

#include "../utils/kstdlib.h"
#include "../memory/paging.h"
#include "../rlyeh/rlyeh.h"
#include "../utils/collections/array.h"

#define FB_COLOR_MODE_EGA  0
#define FB_COLOR_MODE_IDXC 1
#define FB_COLOR_MODE_RGB  2

uint8_t mode;
uint8_t fb_color_mode;
static char* framebuffer;
static uint32_t w, h, fbpitch;
static array_t* bootimg_bank;

extern uint16_t* text_mode_video_memory;

void initialize_bootimg_bank() {
	bootimg_bank = create_array();
	if (bootimg_bank == NULL) {
		error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &initialize_bootimg_bank);
	}

	path_element_t* dir = get_path("bootimg");
	if (dir == NULL || dir->type == PE_FILE) {
		error(ERROR_INITRD_ERROR, INITRD_ERROR_INVALID_FILE, INITRD_IF_BOOTIMG_NOT_DIR, &dir);
	}

	for (uint32_t i=0; i<array_get_size(dir->element.dir->path_el_array); i++) {
		path_element_t* f = array_get_at(dir->element.dir->path_el_array, i);
		if (f->type == PE_DIR)
			continue; // skip directories

		char* extension = get_extension(f->name);
		image_t* image = NULL;

		// TODO add other types
		if (strcmp(extension, "bmp")==0) {
			image = load_bmp(get_data(f->element.file), f->element.file->size);
		}

		if (image != NULL) {
			array_push_data(bootimg_bank, image);
		}
	}
}

void initialize_grx(struct multiboot_info* mb) {
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

	framebuffer = (char*)physical_to_virtual(mb->framebuffer_addr);
	w = mb->framebuffer_width;
	h = mb->framebuffer_height;
	fbpitch = mb->framebuffer_pitch;

	if (mb->framebuffer_type == 2) {
		// EGA text mode, standard MODE_TEXT
		fb_color_mode = FB_COLOR_MODE_EGA;
		text_mode_video_memory = (uint16_t*)framebuffer;
		return;
	}

	mode = MODE_GRAPHICS;
	fb_color_mode = mb->framebuffer_type == 0 ? FB_COLOR_MODE_IDXC : FB_COLOR_MODE_RGB;

	initialize_font(mb);
	initialize_bootimg_bank();
}

uint32_t grx_get_height() {
	return h;
}

uint32_t grx_get_width() {
	return w;
}
