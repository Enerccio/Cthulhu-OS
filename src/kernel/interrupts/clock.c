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
 * clock.c
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: ticker implementation
 */

#include "../interrupts/clock.h"

#define CURRENT_YEAR        2015                            // Change this each year!
int century_register = 0x00;                                // Set by ACPI table parsing code if possible
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

enum {
	cmos_address = 0x70,
	cmos_data    = 0x71
};

/**
 * Returns if update is in progress.
 */
int get_update_in_progress_flag() {
	outb(cmos_address, 0x0A);
	return (inb(cmos_data) & 0x80);
}

/**
 * Returns value from RTC register.
 */
unsigned char get_RTC_register(int reg) {
	outb(cmos_address, reg);
	return inb(cmos_data);
}

/**
 * Determines number of days from specified date.
 */
intmax_t days_from_civil(int32_t y, uint8_t m, uint8_t d) {
	y -= m <= 2;
	int32_t era = (y >= 0 ? y : y-399) / 400;
	unsigned int yoe = (y - era * 400);
	unsigned int doy = (153*(m+(m>2 ? -3 : 9)) + 2) / 5 + d - 1;
	unsigned int doe = yoe * 365 + yoe/4 - yoe/100 + doy;
	return era * 146097 + ((int)doe) - 719468;
}

/** Stores clock seconds in unix time */
volatile uintmax_t clock_s;
/** Stores ms of current ticker */
volatile uintmax_t clock_ms;

/**
 * Ticker callback for ISR.
 */
void timer_tick(uint64_t error_code, registers_t* r) {
	clock_ms += 1;
	if (clock_ms >= 1000){
		++clock_s;
		clock_ms -= 1000;
	}
}

/**
 * Busy waits for X milliseconds.
 *
 * TODO: Fix it, might not be correct
 */
void busy_wait_milis(size_t milis){
	uintmax_t oclocks = clock_s;
	uintmax_t oclockms = clock_ms;
	while (milis > 0){
		uintmax_t clocks = clock_s;
		uintmax_t clockms = clock_ms;

		uintmax_t diff_s = clocks-oclocks;
		intmax_t diff_ms = clockms-oclockms;

		size_t totaldiff = (diff_s*1000)+diff_ms;
		if (totaldiff > milis)
			return;
		else
			milis -= totaldiff;

		oclocks=clocks;
		oclockms=clockms;
	}
}

/**
 * Returns current unix time.
 */
uint64_t get_unix_time() {
	return (uint64_t)clock_s;
}

/**
 * Initializes ticker.
 *
 * Sets up IRQ0 and then enables interrupts.
 */
void initialize_ticker() {
	clock_s = (((uintmax_t)days_from_civil(year, month, day)) * (3600*24)) +
			(hour*3600) + (minute*60) + second;
	clock_ms = 0;

	register_interrupt_handler(IRQ0, &timer_tick);

	uint32_t divisor = 1193180 / 1193;
	outb(0x43, 0x36);
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor>>8) & 0xFF);

	outb(0x40, l);
	outb(0x40, h);

	ENABLE_INTERRUPTS();
}

/**
 * Initializes clock module
 *
 * Calls CMOS for time data, converts it from date to unix time,
 * then initializes ticker.
 */
void initialize_clock() {
	unsigned char century;
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char last_century;
	unsigned char registerB;

	// Note: This uses the "read registers until you get the same values twice in a row" technique
	      //       to avoid getting dodgy/inconsistent values due to RTC updates

	while (get_update_in_progress_flag())
		;                // Make sure an update isn't in progress

	second = get_RTC_register(0x00);
	minute = get_RTC_register(0x02);
	hour = get_RTC_register(0x04);
	day = get_RTC_register(0x07);
	month = get_RTC_register(0x08);
	year = get_RTC_register(0x09);

	if(century_register != 0) {
		century = get_RTC_register(century_register);
	}

	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		last_century = century;

		while (get_update_in_progress_flag())
			;           // Make sure an update isn't in progress

		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);

		if(century_register != 0) {
			  century = get_RTC_register(century_register);
		}
	} while((last_second != second) || (last_minute != minute) || (last_hour != hour) ||
			   (last_day != day) || (last_month != month) || (last_year != year) ||
			   (last_century != century));

	registerB = get_RTC_register(0x0B);

	// Convert BCD to binary values if necessary

	if (!(registerB & 0x04)) {
		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
		if(century_register != 0) {
			  century = (century & 0x0F) + ((century / 16) * 10);
		}
	}

	// Convert 12 hour clock to 24 hour clock if necessary

	if (!(registerB & 0x02) && (hour & 0x80)) {
		hour = ((hour & 0x7F) + 12) % 24;
	}

	// Calculate the full (4-digit) year

	if(century_register != 0) {
		year += century * 100;
	} else {
		year += (CURRENT_YEAR / 100) * 100;
		if(year < CURRENT_YEAR) year += 100;
	}

	initialize_ticker();
}
