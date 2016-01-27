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

#define NO_RECEIVER_PROCESS_SPECIFIED __UINT64_MAX__
#define NO_RECEIVER_THREAD_SPECIFIED  __TID_MAX__

#define MESSAGE_BODY_SIZE ((0x200000)-sizeof(message_header_t))

#define MESSAGE_MAGIC (0x86454D4D)

typedef struct message_target_container {
    pid_t target[64];
} message_target_container_t;

typedef struct message_header {
    pid_t target_process;
    struct {
        uint64_t no_target  : 1;
        uint64_t broadcast  : 1;
        uint64_t group_cast : 1;
        uint64_t group      : 8;
        uint64_t self_state : 2; // 00 - block until reply, 01 - block all threads until reply,
                                 // 10 block other threads until reply, 11 - reserved
        uint64_t deli_state : 2; // 00 - normal message, 01 - interrupt any,
                                 // 10 - interrupt one by tid specified in target_thread
        uint64_t ignore_im  : 1;
        uint64_t reserved   : 48;
    } flags;
    uint64_t target_thread;
    uint64_t magic;
    uint64_t checksum;
} message_header_t;

typedef struct message {
    message_header_t header;
    char data[MESSAGE_BODY_SIZE];
} message_t;

message_t* get_free_message();
int		   send_message(message_t* message);
int		   receive_message(void* bodyptr);

#ifdef __cplusplus
}
#endif
