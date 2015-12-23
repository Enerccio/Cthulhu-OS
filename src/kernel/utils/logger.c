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

typedef enum {

    MESSAGE, WARNING, ERROR,

} log_level_t;

void log(log_level_t log_level, const char* message) {

    // write message that we are in the kernel

    // display log level
    switch (log_level) {
    case WARNING:
        kd_cwrite("WARNING: ", 0, 6);
        break;
    case ERROR:
        kd_cwrite("ERROR: ", 0, 4);
        break;
    default:
        kd_cwrite("MESSAGE: ", 0, 15);
        break;
    }

    kd_write(message);
    kd_put('\n');
}

void log_msg(const char* message) {
    log(MESSAGE, message);
}

void log_warn(const char* message) {
    log(WARNING, message);
}

void log_err(const char* message) {
    log(ERROR, message);
}

void vlog(log_level_t log_level, const char* message, va_list l) {

    // write message that we are in the kernel

    // display log level
    switch (log_level) {
    case WARNING:
        kd_cwrite("WARNING: ", 0, 6);
        break;
    case ERROR:
        kd_cwrite("ERROR: ", 0, 4);
        break;
    default:
        kd_cwrite("MESSAGE: ", 0, 15);
        break;
    }

    vprintf(message, l);
    printf("\n");
}

void vlog_msg(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(MESSAGE, message, a_list);
    va_end(a_list);
}

void vlog_warn(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(WARNING, message, a_list);
    va_end(a_list);
}

void vlog_err(const char* message, ...) {
    va_list a_list;
    va_start(a_list, message);
    vlog(ERROR, message, a_list);
    va_end(a_list);
}
