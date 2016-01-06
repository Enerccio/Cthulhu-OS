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
 * framebuffer.cc
 *  Created on: Jan 6, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "../../grx/grx.h"
#include "../../grx/image.h"

ruint_t dev_fb_get_height(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_height();
}

ruint_t dev_fb_get_width(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_width();
}

ruint_t dev_fb_update(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	flush_buffer();
	return DS_ERROR_SUCCESS;
}

ruint_t dev_fb_write(registers_t* r, ruint_t bytes, ruint_t x, ruint_t y, ruint_t w, ruint_t h) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	image_t image;
	image.image_data = (uint8_t*)bytes;
	image.image_type = IMAGE_RGB;
	image.w = (uint32_t)w;
	image.h = (uint32_t)h;

	blit(&image, (uint32_t)x, (uint32_t)y);

	return DS_ERROR_SUCCESS;
}

ruint_t dev_fb_clear(registers_t* r, ruint_t red, ruint_t green, ruint_t blue) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	clear_screen(red, green, blue);

	return DS_ERROR_SUCCESS;
}

ruint_t dev_fb_putpixel(registers_t* r, ruint_t* color, ruint_t x, ruint_t y) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	image_t image;
	image.image_data = (uint8_t*)color;
	image.image_type = IMAGE_RGB;
	image.w = 1;
	image.h = 1;

	blit(&image, (uint32_t)x, (uint32_t)y);

	return DS_ERROR_SUCCESS;
}
