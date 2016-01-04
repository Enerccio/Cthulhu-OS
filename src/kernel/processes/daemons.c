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
 * daemons.c
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "daemons.h"

extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);

ruint_t __daemon_registration_lock;
hash_table_t* dr_table;

uint64_t register_daemon_service(uint64_t process, const char* service, bool overwrite_old_service_provider) {
    proc_spinlock_lock(&__daemon_registration_lock);

    if (table_contains(dr_table, (void*)service) && !overwrite_old_service_provider) {
        return DAEMON_NOT_REGISTERED;
    }
    table_set(dr_table, (void*)service, (void*)(uintptr_t)process);
    uint64_t cdp = table_get(dr_table, (void*)service);

    proc_spinlock_unlock(&__daemon_registration_lock);

    return cdp;
}

void initialize_daemon_services() {
    __daemon_registration_lock = 0;
    dr_table = create_string_table();
}
