#include "textinput.h"

uint16_t* video_memory = (uint16_t*) 0xB8000;
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

static inline void scroll() {

	uint8_t attributeByte = (0 /*black*/<< 4) | (15 /*white*/& 0x0F);
	uint16_t blank = 0x20 | (attributeByte << 8);

	if (cursor_y >= 25) {
		int i;
		for (i = 0 * 80; i < 24 * 80; i++) {
			video_memory[i] = video_memory[i + 80];
		}

		for (i = 24 * 80; i < 25 * 80; i++) {
			video_memory[i] = blank;
		}
		cursor_y = 24;
	}
}


static inline void move_cursor() {
	uint16_t cursorLocation = cursor_y * 80 + cursor_x;
	outb(0x3D4, 14);
	outb(0x3D5, cursorLocation >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, (uint8_t) cursorLocation);
}


void kd_put(char c) {
	kd_cput(c, 0, 7);
}


void kd_cput(char c, uint8_t backColour, uint8_t foreColour) {
	uint8_t attributeByte = (backColour << 4) | (foreColour & 0x0F);

	uint16_t attribute = attributeByte << 8;
	uint16_t* location;

	if (c == 0x08 && cursor_x) {
		if (cursor_x)
			cursor_x--;
	} else if (c == 0x09) {
		cursor_x = (cursor_x + 8) & ~(8 - 1);
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c >= ' ') {
		location = video_memory + (cursor_y * 80 + cursor_x);
		*location = c | attribute;
		cursor_x++;
	}

	if (cursor_x >= 80) {
		cursor_x = 0;
		cursor_y++;
	}

	scroll();
	move_cursor();
}


void kd_clear() {
	kd_cclear(0);
}


void kd_cclear(uint8_t backColour) {
	uint8_t attributeByte = (backColour << 4) | (15 & 0x0F);
	uint16_t blank = 0x20 | (attributeByte << 8);

	int i;
	for (i = 0; i < 80 * 25; i++) {
		video_memory[i] = blank;
	}

	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}


void kd_write(const char* c) {
	kd_cwrite(c, 0, 7);
}


void kd_cwrite(const char* string, uint8_t backColour, uint8_t foreColour) {
	char c;
	char* it = (char*) string;
	while ((c = *it++)) {
		kd_cput(c, backColour, foreColour);
	}
}


void kd_newl() {
	kd_put('\n');
}

void kd_setxy(uint8_t x, uint8_t y) {
	cursor_x = x;
	cursor_y = y;
	move_cursor();
}

void kd_write_hex(uint32_t number) {
	kd_cwrite_hex(number, 0, 7);
}

void kd_write_hex64(uint64_t number) {
	kd_cwrite_hex(number, 0, 7);
}

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


static inline void __write_hex_c(uint8_t num, uint8_t bgcolor, uint8_t fgcolor) {
	__write_hex_c8(num / 16, bgcolor, fgcolor);
	__write_hex_c8(num % 16, bgcolor, fgcolor);
}


void kd_cwrite_hex(uint32_t number, uint8_t bgcolor, uint8_t fgcolor) {
	kd_cwrite("0x", bgcolor, fgcolor);

	__write_hex_c(number / (1 << 24), bgcolor, fgcolor);
	__write_hex_c(number / (1 << 16), bgcolor, fgcolor);
	__write_hex_c(number / (1 << 8), bgcolor, fgcolor);
	__write_hex_c(number % 256, bgcolor, fgcolor);
}

void kd_cwrite_hex64(uint64_t number, uint8_t bgcolor, uint8_t fgcolor) {
	kd_cwrite("0x", bgcolor, fgcolor);

	__write_hex_c(number / (1ULL << 56), bgcolor, fgcolor);
	__write_hex_c(number / (1ULL << 48), bgcolor, fgcolor);
	__write_hex_c(number / (1ULL << 40), bgcolor, fgcolor);
	__write_hex_c(number / (1ULL << 32), bgcolor, fgcolor);

	__write_hex_c(number / (1 << 24), bgcolor, fgcolor);
	__write_hex_c(number / (1 << 16), bgcolor, fgcolor);
	__write_hex_c(number / (1 << 8), bgcolor, fgcolor);
	__write_hex_c(number % 256, bgcolor, fgcolor);
}

