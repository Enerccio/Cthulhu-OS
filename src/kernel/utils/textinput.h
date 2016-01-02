/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * textinput.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: basic vga text io
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef REGISTER_UINT_TYPE
#define REGISTER_UINT_TYPE uint64_t
#endif
typedef REGISTER_UINT_TYPE ruint_t;

#ifndef PHYSICAL_UINT_TYPE
#define PHYSICAL_UINT_TYPE uintptr_t
#endif
typedef PHYSICAL_UINT_TYPE puint_t;

#include "../grx/grx.h"

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);

/**
 * Clears screen with black color.
 */
void kd_clear();

/**
 * Clears screen with selected color.
 */
void kd_cclear(uint8_t bg_color);

/**
 * Outputs one byte of data to screen with gray color and black background.
 */
void kd_put(char c);

/**
 * Outputs one colored character.
 *
 * If KERNEL_DEBUG_MODE is set, outputs same byte into com1.
 */
void kd_cput(char c, uint8_t bg_color, uint8_t fg_color);

/**
 * Writes string on screen with gray color on black background.
 */
void kd_write(const char* c);

/**
 * Writes string on screen with specified colors.
 */
void kd_cwrite(const char* c, uint8_t bg_color, uint8_t fg_color);

/**
 * Sets position of cursor.
 */
void kd_setxy(uint8_t x, uint8_t y);

/**
 * Writes 32bit hex number with gray color and black background.
 */
void kd_write_hex(uint32_t number);

/**
 * Writes 64bit hex number with gray color and black background.
 */
void kd_write_hex64(ruint_t number);

/**
 * Writes 32 bit hex number.
 */
void kd_cwrite_hex(uint32_t number, uint8_t bg_color, uint8_t fg_color);

/**
 * Writes 64 bit hex number.
 */
void kd_cwrite_hex64(ruint_t number, uint8_t bgcolor, uint8_t fgcolor);

/**
 * Emits new line.
 */
void kd_newl();
