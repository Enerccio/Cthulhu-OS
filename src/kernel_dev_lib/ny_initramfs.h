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
 * ny_initramfs.h
 *  Created on: Dec 30, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "ny_stddef.h"
#include "ny_commons.h"
#include "devsys.h"
#include <stdnoreturn.h>

#define E_IFS_INITRAMFS_GONE -1
#define E_IFS_ACTION_SUCCESS  0
#define E_IFS_PATH_INCORRECT  1
#define E_IFS_NOT_A_DECTYPE   2

typedef enum entry_type {
	et_dir, et_file
} entry_type_t;

typedef struct initramfs_entry {
	entry_type_t type;
	size_t       num_ent_or_size;
	char         name[256];
} initramfs_entry_t;

typedef struct ifs_directory {
	initramfs_entry_t entry;
	char** entries;
} ifs_directory_t;

typedef struct ifs_file {
	initramfs_entry_t entry;
	char* file_contents;
} ifs_file_t;

int get_directory(const char* path, ifs_directory_t* dir);
int get_file(const char* path, ifs_file_t* file);
int execve_ifs(const char* ifs_path, char** argv, char** envp);
