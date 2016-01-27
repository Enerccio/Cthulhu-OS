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
 * daemons.h
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include <ds/hmap.h>

typedef struct continuation continuation_t;

#define DAEMON_NOT_REGISTERED -1

// List of internal services
// These are provided internally by daemons
// other daemons register their own services by their name and then inform these daemon
//  services by their name and they communicate
#define SERVICE_DDM "::device-driver-manager"
#define SERVICE_VFS "::service::internal::vfs"
#define SERVICE_PORT "::service::internal::port"
#define SERVICE_KEYBOARD "::service::internal::keyboard"
#define SERVICE_MOUSE "::service::internal::mouse"
#define SERVICE_FRAMEBUFFER "::service::internal::framebuffer"
#define SERVICE_USERS "::service::internal::users"

pid_t register_daemon_service(pid_t process, const char* service,
        bool overwrite_old_service_provider, continuation_t* c);

bool daemon_registered(const char* service);
bool is_daemon_process(pid_t process, const char* service);

void initialize_daemon_services();
