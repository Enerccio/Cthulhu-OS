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
 * rlyeh.h
 *  Created on: Dec 30, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include "../utils/collections/array.h"
#include "../../kernel_dev_lib/ny_initramfs.h"

#define INITRD_ERROR_NO_INITRD		(1)
#define INITRD_ERROR_WRONG_HEADER	(2)
#define INITRD_ERROR_NOMEM			(3)
#define INITRD_ERROR_INVALID_FILE	(4)

#define INITRD_IF_DIR_AS_FILE		(0)
#define INITRD_IF_BOOTIMG_NOT_DIR	(1)

#define PE_DIR  (0)
#define PE_FILE (1)

typedef struct dir_entry {
	array_t* path_el_array;
} dir_entry_t;

typedef struct file_entry {
	size_t size;
	size_t offset;
} file_entry_t;

typedef struct path_element {
	char* name;
	uint8_t type;
	union {
		dir_entry_t* dir;
		file_entry_t* file;
	} element;
} path_element_t;

void init_initramfs(struct multiboot_info* info);

path_element_t* get_root();

path_element_t* get_path(const char* path);

uint8_t* get_data(file_entry_t* file);

void free_initramfs();
