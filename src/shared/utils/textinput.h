#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((regparm(2)))
extern void outb(uint16_t port, uint8_t value);
__attribute__((regparm(1)))
extern uint8_t inb(uint16_t port);
__attribute__((regparm(1)))
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
