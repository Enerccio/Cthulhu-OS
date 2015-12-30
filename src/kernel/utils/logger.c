/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
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
 * logger.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: minimal kernel logger functionality
 */
#include "logger.h"
#include "../utils/collections/array.h"

typedef enum {
    MESSAGE, WARNING, ERROR,
} log_level_t;

array_t* boot_log;

typedef struct log_entry {
	log_level_t ll;
	char message[255];
} log_entry_t;

extern bool __ports_initialized;
extern bool __print_initialized;
extern void write_byte_com(uint8_t com, uint8_t data);

void initialize_logger() {
	boot_log = create_array();
}

static inline void print_to_com(char* ch) {
	char c;
	while ((c = *ch++) != '\0') {
		if (c == '\n')
	        write_byte_com(0, '\r');
	    write_byte_com(0, c);
	}
}

/**
 * Logs simple message with log level.
 */
void log(log_level_t log_level, const char* message) {

    // write message that we are in the kernel
	log_entry_t* le = malloc(sizeof(log_entry_t));
	le->ll = log_level;

    // display log level
    switch (log_level) {
    case WARNING:
    	if (__ports_initialized) {
    		print_to_com("WARNING: ");
    	}
    	if (__print_initialized) {
    		kd_cwrite("WARNING: ", 0, 6);
    	}
        break;
    case ERROR:
    	if (__ports_initialized) {
			print_to_com("ERROR: ");
		}
		if (__print_initialized) {
			kd_cwrite("ERROR: ", 0, 4);
		}
        break;
    default:
    	if (__ports_initialized) {
			print_to_com("MESSAGE: ");
		}
		if (__print_initialized) {
			kd_cwrite("MESSAGE: ", 0, 15);
		}
        break;
    }

    if (__ports_initialized) {
		print_to_com((char*)message);
		print_to_com("\n");
	}
	if (__print_initialized) {
	    kd_write(message);
	    kd_put('\n');
	}

	strncpy(le->message, message, 254);
	le->message[254] = '\0';

	array_push_data(boot_log, le);
}

/**
 * Logs simple message with MESSAGE level.
 */
void log_msg(const char* message) {
    log(MESSAGE, message);
}

/**
 * Logs simple message with WARNING level.
 */
void log_warn(const char* message) {
    log(WARNING, message);
}

/**
 * Logs simple message with ERROR level.
 */
void log_err(const char* message) {
    log(ERROR, message);
}

/**
 * Advanced logging, using prinf (can only be called
 * after printf is available).
 */
void vlog(log_level_t log_level, const char* message, va_list l) {
    // write message that we are in the kernel
    // display log level
    char tmpbuf[256];
    vsnprintf(tmpbuf, 255, message, l);
    log(log_level, tmpbuf);
}

/**
 * Logs advance message with MESSAGE level.
 */
void vlog_msg(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(MESSAGE, message, a_list);
    va_end(a_list);
}

/**
 * Logs advance message with WARN level.
 */
void vlog_warn(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(WARNING, message, a_list);
    va_end(a_list);
}

/**
 * Logs advance message with ERROR level.
 */
void vlog_err(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(ERROR, message, a_list);
    va_end(a_list);
}
