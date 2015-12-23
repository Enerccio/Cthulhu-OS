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

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);

void kd_clear();
void kd_cclear(uint8_t bg_color);
void kd_put(char c);
void kd_cput(char c, uint8_t bg_color, uint8_t fg_color);
void kd_write(const char* c);
void kd_cwrite(const char* c, uint8_t bg_color, uint8_t fg_color);
void kd_setxy(uint8_t x, uint8_t y);
void kd_write_hex(uint32_t number);
void kd_write_hex64(uint64_t number);
void kd_cwrite_hex(uint32_t number, uint8_t bg_color, uint8_t fg_color);
void kd_cwrite_hex64(uint64_t number, uint8_t bgcolor, uint8_t fgcolor);
void kd_newl();
