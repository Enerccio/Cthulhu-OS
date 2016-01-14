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
 * messages.h
 *  Created on: Jan 13, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "ct_commons.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ipc_pointer_t;

#define NO_RECEIVER_PROCESS_SPECIFIED __UINT64_MAX__
#define NO_RECEIVER_THREAD_SPECIFIED  __TID_MAX__

/** system messages */
typedef enum smess_type {
	create_process
} smess_type_t;

typedef enum create_process_mode {
	initramfs, vfs
} create_process_mode_t;
typedef struct create_process_message {
	const char*  path;
	const char** argv;
	const char** envv;
	int argc;
	create_process_mode_t mode;

	// TODO: add more later
} create_process_message_t;

typedef enum message_type { system_message, simple_message, large_message } message_type_h;

typedef struct message_header {
	message_type_h mtype;
	message_main_type_t mmtype;
	pid_t process_id;
	bool gift_tickets;
	bool await_reply;
} message_header_t;

typedef struct system_message {
	message_header_t header;
	smess_type_t message_type;
	union {
		create_process_message_t cpm;
	} message_contents;
} system_message_t;

/* for simple/large messages */
typedef enum message_main_type {
    targetted_message,
    broadcast_message,
    daemon_message,
    continuation_message,
} message_main_type_t;

ruint_t send_message(message_header_t* mh);

#ifdef __cplusplus
}
#endif
