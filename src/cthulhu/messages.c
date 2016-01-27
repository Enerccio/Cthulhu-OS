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
 * messages.c
 *  Created on: Jan 14, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "messages.h"

message_t* get_free_message() {
	message_t* mp;
	int error = 0;
	do {
		error = sys_1arg(SYS_GET_FMESSAGE_BLOCK, (ruint_t)&mp);
	} while (error != 0 && error == ETRYAGAIN);
	if (error != 0) {
		return NULL;
	} else {
		return mp;
	}
}

uint64_t compute_message_checksum(message_t* message) {
	uint64_t cm = 0;
	uint8_t* mbytes = (uint8_t*)message;
	for (unsigned int i=0; i<sizeof(message_t); i++) {
		cm += mbytes[i];
	}
	return 16 - (cm % 16);
}

int	send_message(message_t* message) {
	message->header.checksum = compute_message_checksum(message);
	int error = 0;
	do {
		error = sys_1arg(SYS_SEND_MESSAGE, (ruint_t)message);
	} while (error != 0 && error == ETRYAGAIN);
	return error;
}

int receive_message(void* bodyptr) {
	return sys_1arg(SYS_RECEIVE_MESSAGE, (ruint_t)bodyptr);
}
