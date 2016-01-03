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
 * ipc.h
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include "daemons.h"

#include <sys/types.h>

typedef void* ipc_pointer_t;

#define NO_RECEIVER_PROCESS_SPECIFIED __UINT64_MAX__
#define NO_RECEIVER_THREAD_SPECIFIED  __TID_MAX__

typedef enum message_type {
    targetted_message,
    broadcast_message,
    daemon_message,
    continuation_message,
} message_type_t;

typedef struct message {
    message_type_t mtype;

    uint64_t sender_process;
    tid_t    sender_thread;
    uint64_t receiver_process;

    ruint_t message_inc_id;
    ruint_t split_message_order;
    bool     has_next_message;

    /* Body is special pointer because it points to physical memory */
    /* However, it should be kernel readable identity map pointer */
    /* with size of 4096 bytes (one page) */
    ipc_pointer_t body;
    uint16_t body_used_bytes;

    /** Used for message managing */
    ruint_t __message_lock;
    uint32_t __message_ref_count;
} message_t;
