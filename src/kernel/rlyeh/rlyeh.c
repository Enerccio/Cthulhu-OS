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
 * rlyeh.c
 *  Created on: Dec 30, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "rlyeh.h"
#include "../utils/rsod.h"
#include "../utils/kstdlib.h"
#include "../memory/paging.h"

#define MAGIC_1 0xDE
#define MAGIC_2 0xAF
#define MAGIC_3 0xD0
#define MAGIC_4 0x55

typedef struct cthulhuos_initrd {
	uint8_t  magic[4];
	uint32_t nument;
	uint32_t dataoffset;
	uint8_t  header;
} cthulhu_initr_t;

typedef struct lfile_info {
	uint32_t offset;
	uint32_t fsize;
	char     fpath[256];
} lfile_info_t;

cthulhu_initr_t* initrd_module;
size_t initrd_size;
path_element_t* root;

void delete_dir_entry(dir_entry_t* de);
void delete_file_entry(file_entry_t* fe);

void delete_entry(path_element_t* pe) {
	if (pe->type == PE_DIR) {
		delete_dir_entry(pe->element.dir);
	} else {
		delete_file_entry(pe->element.file);
	}
	free(pe->name);
	free(pe);
}

void delete_dir_entry(dir_entry_t* de) {
	for (uint32_t i=0; i<array_get_size(de->path_el_array); i++) {
		delete_entry((path_element_t*)array_get_at(de->path_el_array, i));
	}
	destroy_array(de->path_el_array);
	free(de);
}

void delete_file_entry(file_entry_t* fe) {
	free(fe);
}

void free_initramfs() {
	delete_entry(root);
	deallocate_starting_address((uint64_t)initrd_module, initrd_size);
	root = NULL;
}

void mk_dir(path_element_t** pe_ptr, char* name) {
	path_element_t* pe;
	pe = malloc(sizeof(path_element_t));
	if (pe == NULL) {
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NOMEM, (uint64_t)pe_ptr, &mk_dir);
	}

	size_t len = strlen(name);
	char* nname = (char*)malloc(len+1);
	memcpy(nname, name, len+1);

	pe->type = PE_DIR;
	pe->name = nname;
	pe->element.dir = malloc(sizeof(dir_entry_t));
	if (pe->element.dir == NULL) {
		free(pe);
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NOMEM, (uint64_t)pe_ptr, &mk_dir);
	}

	pe->element.dir->path_el_array = create_array();
	if (pe->element.dir->path_el_array == NULL) {
		free(pe);
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NOMEM, (uint64_t)pe_ptr, &mk_dir);
	}

	*pe_ptr = pe;
}

void mk_file(path_element_t** pe_ptr, char* name, size_t fsize, size_t offset) {
	path_element_t* pe;
	pe = malloc(sizeof(path_element_t));
	if (pe == NULL) {
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NOMEM, (uint64_t)pe_ptr, &mk_dir);
	}

	size_t len = strlen(name);
	char* nname = (char*)malloc(len+1);
	memcpy(nname, name, len+1);

	pe->type = PE_FILE;
	pe->name = nname;
	pe->element.file = malloc(sizeof(file_entry_t));
	if (pe->element.file == NULL) {
		free(pe);
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NOMEM, (uint64_t)pe_ptr, &mk_dir);
	}

	pe->element.file->size = fsize;
	pe->element.file->offset = offset;

	*pe_ptr = pe;
}

bool find_by_name(void* element, void* passed_data) {
	path_element_t* pe = (path_element_t*) element;
	return strcmp(pe->name, (char*)passed_data) == 0;
}

void load_header(uint8_t* header, uint32_t nument) {
	for (uint32_t i=0; i<nument; i++) {
		lfile_info_t* lf = (lfile_info_t*) (header+(i*sizeof(lfile_info_t)));

		char* pe = strtok(lf->fpath, "/");
		path_element_t* pel = root;

		do {
			if (pel->type == PE_FILE) {
				error(ERROR_INITRD_ERROR, INITRD_ERROR_INVALID_FILE, INITRD_IF_DIR_AS_FILE, pel);
			}
			path_element_t* new_pe = (path_element_t*) array_find_by_pred(pel->element.dir->path_el_array,
					find_by_name, pe);
			if (new_pe == NULL) {
				mk_dir(&new_pe, pe);
				array_push_data(pel->element.dir->path_el_array, new_pe);
			}
			pel = new_pe;
		} while ((pe = strtok(NULL, "/")) != NULL);

		// pel should now be "dir" entry, but it is file, so we free inner dir entry and change type
		delete_dir_entry(pel->element.dir);
		pel->type = PE_FILE;
		pel->element.file = malloc(sizeof(file_entry_t));
		pel->element.file->size = lf->fsize;
		pel->element.file->offset = initrd_module->dataoffset + lf->offset;
	}
}

void init_initramfs(struct multiboot_info* info) {
	initrd_module = (cthulhu_initr_t*)get_module(info, "initrd", &initrd_size, false, false);
	if (initrd_module == NULL) {
		error(ERROR_INITRD_ERROR, INITRD_ERROR_NO_INITRD, 0, info);
	}

	if (initrd_module->magic[0] != MAGIC_1 ||
			initrd_module->magic[1] != MAGIC_2 ||
			initrd_module->magic[2] != MAGIC_3 ||
			initrd_module->magic[3] != MAGIC_4) {
		error(ERROR_INITRD_ERROR, INITRD_ERROR_WRONG_HEADER, *(uint32_t*)initrd_module->magic, &initrd_module);
	}

	mk_dir(&root, "");
	load_header(&initrd_module->header, initrd_module->nument);
}

path_element_t* get_root() {
	return root;
}

path_element_t* get_path(const char* path) {
	char* pe = strtok((char*)path, "/");
	path_element_t* pel = root;

	do {
		path_element_t* new_pe = (path_element_t*) array_find_by_pred(pel->element.dir->path_el_array,
				find_by_name, pe);
		if (new_pe == NULL) {
			return NULL;
		}
		pel = new_pe;
	} while ((pe = strtok(NULL, "/")) != NULL);

	return pel;
}

uint8_t* get_data(file_entry_t* file) {
	return (uint8_t*) (file->offset + (uint64_t)initrd_module);
}
