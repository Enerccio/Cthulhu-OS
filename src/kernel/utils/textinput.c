/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
#include "textinput.h"

#include "../grx/grx.h"
#include "../grx/font.h"

extern volatile uint16_t* text_mode_video_memory;
uint8_t cursor_y = 0;
uint8_t cursor_x = 0;

typedef enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
} vga_color_t;

/**
 * Scrolls the screen by one line.
 */
static inline void scroll() {
    if (mode == MODE_TEXT) {
        uint8_t attributeByte = (0 /*black*/<< 4) | (15 /*white*/& 0x0F);
        uint16_t blank = 0x20 | (attributeByte << 8);

        if (cursor_y >= grx_get_height()+1) {
            unsigned int i;
            for (i = 0 * grx_get_width(); i < grx_get_height() * grx_get_width(); i++) {
                text_mode_video_memory[i] = text_mode_video_memory[i + grx_get_width()];
            }

            for (i = grx_get_height() * grx_get_width(); i < (grx_get_height()+1) * grx_get_width(); i++) {
                text_mode_video_memory[i] = blank;
            }
            cursor_y = grx_get_height();
        }
    } else {
        if (cursor_y >= (grx_get_height()/__font_h)+1) {
            scroll_up(__font_h);
            cursor_y = (grx_get_height()/__font_h);
        }
    }
}

/**
 * Moves cursor to position stored by cursor_y and cursor_x.
 */
static inline void move_cursor() {
    uint16_t cursorLocation = cursor_y * grx_get_width() + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, cursorLocation >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t) cursorLocation);
}

/**
 * Outputs one byte of data to screen with gray color and black background.
 */
void kd_put(char c) {
    kd_cput(c, 0, 7);
}

/**
 * Outputs one colored character.
 *
 * If KERNEL_DEBUG_MODE is set, outputs same byte into com1.
 */
void kd_cput(char c, uint8_t back_color, uint8_t fore_color) {
    if (c == 0x08 && cursor_x) {
        if (cursor_x)
            cursor_x--;
    } else if (c == 0x09) {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else if (c == '\r') {
        flush_buffer();
        cursor_x = 0;
    } else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        flush_buffer();
    } else if (c >= ' ') {
        if (mode == MODE_TEXT) {
            uint8_t attributeByte = (back_color << 4) | (fore_color & 0x0F);
            uint16_t attribute = attributeByte << 8;
            volatile uint16_t* location;
            location = text_mode_video_memory + (cursor_y * grx_get_width() + cursor_x);
            *location = c | attribute;
            cursor_x++;
        } else {
            blit_colored(get_letter(c), cursor_x*__font_w, cursor_y*__font_h, ega[fore_color]);
            cursor_x++;
        }
    }

    if (cursor_x >= 80 && mode == MODE_TEXT) {
        cursor_x = 0;
        cursor_y++;
    } else if (cursor_x >= grx_get_width()/__font_w) {
        cursor_x = 0;
        cursor_y++;
    }

    scroll();
    if (mode == MODE_TEXT) {
        move_cursor();
    }
}

/**
 * Clears screen with black color.
 */
void kd_clear() {
    kd_cclear(0);
}

/**
 * Clears screen with selected color.
 */
void kd_cclear(uint8_t back_color) {
    if (mode == MODE_TEXT) {
        uint8_t attributeByte = (back_color << 4) | (15 & 0x0F);
        uint16_t blank = 0x20 | (attributeByte << 8);

        unsigned int i;
        for (i = 0; i < grx_get_width() * grx_get_height(); i++) {
            text_mode_video_memory[i] = blank;
        }

        cursor_x = 0;
        cursor_y = 0;
        move_cursor();
    } else {
        blit_colored(clear_screen_blit, 0, 0, ega[back_color]);
        flush_buffer();
    }
}

/**
 * Writes string on screen with gray color on black background.
 */
void kd_write(const char* c) {
    kd_cwrite(c, 0, 7);
}

/**
 * Writes string on screen with specified colors.
 */
void kd_cwrite(const char* string, uint8_t back_color, uint8_t fore_color) {
    char c;
    char* it = (char*) string;
    while ((c = *it++)) {
        kd_cput(c, back_color, fore_color);
    }
}

/**
 * Emits new line.
 */
void kd_newl() {
    kd_put('\n');
}

/**
 * Sets position of cursor.
 */
void kd_setxy(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
    move_cursor();
}

/**
 * Writes 32bit hex number with gray color and black background.
 */
void kd_write_hex(uint32_t number) {
    kd_cwrite_hex(number, 0, 7);
}

/**
 * Writes 64bit hex number with gray color and black background.
 */
void kd_write_hex64(ruint_t number) {
    kd_cwrite_hex(number, 0, 7);
}

/**
 * Writes single hex digit.
 */
static inline void __write_hex_c8(uint8_t num, uint8_t bgcolor, uint8_t fgcolor) {
    switch (num) {
    case 10:
        kd_cput('A', bgcolor, fgcolor);
        break;
    case 11:
        kd_cput('B', bgcolor, fgcolor);
        break;
    case 12:
        kd_cput('C', bgcolor, fgcolor);
        break;
    case 13:
        kd_cput('D', bgcolor, fgcolor);
        break;
    case 14:
        kd_cput('E', bgcolor, fgcolor);
        break;
    case 15:
        kd_cput('F', bgcolor, fgcolor);
        break;
    default:
        kd_cput('0' + num, bgcolor, fgcolor);
    }
}

/**
 * Writes byte as hex number.
 */
static inline void __write_hex_c(uint8_t num, uint8_t bgcolor, uint8_t fgcolor) {
    __write_hex_c8(num / 16, bgcolor, fgcolor);
    __write_hex_c8(num % 16, bgcolor, fgcolor);
}

/**
 * Writes 32 bit hex number.
 */
void kd_cwrite_hex(uint32_t number, uint8_t bgcolor, uint8_t fgcolor) {
    kd_cwrite("0x", bgcolor, fgcolor);

    __write_hex_c(number / (1 << 24), bgcolor, fgcolor);
    __write_hex_c(number / (1 << 16), bgcolor, fgcolor);
    __write_hex_c(number / (1 << 8), bgcolor, fgcolor);
    __write_hex_c(number % 256, bgcolor, fgcolor);
}

/**
 * Writes 64 bit hex number.
 */
void kd_cwrite_hex64(ruint_t number, uint8_t bgcolor, uint8_t fgcolor) {
    kd_cwrite("0x", bgcolor, fgcolor);

    if (sizeof(ruint_t) == 8) {
        __write_hex_c(number / (1ULL << 56), bgcolor, fgcolor);
        __write_hex_c(number / (1ULL << 48), bgcolor, fgcolor);
        __write_hex_c(number / (1ULL << 40), bgcolor, fgcolor);
        __write_hex_c(number / (1ULL << 32), bgcolor, fgcolor);
    }

    __write_hex_c(number / (1 << 24), bgcolor, fgcolor);
    __write_hex_c(number / (1 << 16), bgcolor, fgcolor);
    __write_hex_c(number / (1 << 8), bgcolor, fgcolor);
    __write_hex_c(number % 256, bgcolor, fgcolor);
}

