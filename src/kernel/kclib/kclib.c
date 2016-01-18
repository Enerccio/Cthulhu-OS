/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * kclib.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: implementing external definitions for kclib
 */
#include <sys/external.h>
#include <sys/types.h>

#include "../utils/rsod.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "../utils/textinput.h"
#include "../cpus/ipi.h"
#include "../cpus/cpu_mgmt.h"

__noreturn __kclib_assert_failure_k(uint32_t lineno, const char* file, const char* func) {
    error(ERROR_INTERNAL_LIBC, 0, lineno, 0);
}

void* __kclib_heap_start() {
    return (void*)heap_start_address;
}

void*     __kclib_allocate(size_t aamount) {
    uintptr_t ha = heap_start_address;
    if (!allocate(heap_start_address, aamount, true, false))
    	return NULL;
    heap_start_address += ALIGN_UP(aamount); // fix deallocation
    return (void*)ha;
}

void      __kclib_deallocate(uintptr_t afrom, size_t aamount) {
    deallocate(afrom, aamount);
}

void*     __kclib_open_std_stream(uint8_t request_mode) {
    return (void*)(uintptr_t)request_mode;
}

ptrdiff_t  __kclib_send_data(void* stream, uint8_t* array, size_t buffer_size) {
    if (stream == ((void*)1)) {
        for (size_t i=0; i<buffer_size; i++) {
            kd_put(array[i]);
        }
        return buffer_size;
    }
    return 0;
}

ptrdiff_t  __kclib_read_data(void* stream, uint8_t* buffer, size_t read_amount) {
    return 0;
}

extern uint64_t get_unix_time();
clock_t __kclib_clock() {
    return get_unix_time() * CLOCKS_PER_SEC;
}

mtx_id_t __kclib_get_mutex_global_identifier() {
    return 0; // all mutexes in kernel have same identifier
}

extern bool multiprocessing_ready;

extern uint8_t get_local_apic_id();
tid_t __kclib_get_tid() {
    if (multiprocessing_ready)
        return get_local_apic_id();
    return 0;
}

extern void wait_until_activated();
void __kclib_mutex_halt(mtx_id_t __asked_mutex) {
    if (multiprocessing_ready)
        wait_until_activated(WAIT_KERNEL_MUTEX);
}

extern void broadcast_ipi_message(bool self, uint8_t message_type, ruint_t message, ruint_t message2,
        registers_t* r);
void __kclib_mutex_unlocked(mtx_id_t __asked_mutex) {
    if (multiprocessing_ready)
        broadcast_ipi_message(false, IPI_WAKE_UP_FROM_WUA, WAIT_KERNEL_MUTEX, 0, NULL);
}

void __kclib_mutex_locked(mtx_id_t __asked_mutex) {

}
