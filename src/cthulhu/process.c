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
 * process.c
 *  Created on: Jan 13, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "process.h"

#include <errno.h>

pid_t getpid() {
	int error = 0;
	pid_t pid = (pid_t) sys_0arg_e(SYS_GET_PID, (ruint_t*)&error);
	if (error != 0) {
		errno = error;
	}
	return pid;
}

int create_process_s_cb(const char* path, int argc, char** argv,
		char** envp, bool waitfr, cp_cb_t callback, void* data) {
	system_message_t sm;

	sm.header.mtype = system_message;
	sm.header.await_reply = waitfr;
	sm.message_type = create_process;
	sm.message_contents.cpm.argc = argc;
	sm.message_contents.cpm.argv = argv;
	sm.message_contents.cpm.envp = envp;
	sm.message_contents.cpm.path = path;
	sm.message_contents.cpm.process_priority = (uint8_t) sys_0arg(SYS_GET_CTHREAD_PRIORITY);

	int error;
	if (callback != NULL) {
		if ((error=callback(&sm, data))!=0)
			return error;
	}

	return (int) send_message(&sm.header);
}
