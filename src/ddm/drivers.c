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
 * drivers.c
 *  Created on: Jan 27, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "drivers.h"

HMAP_FUNCDEFS(string, string)
dlist_entry_map* drivers = NULL;

void load_drivers(char* path) {
    if (drivers == NULL) {
        drivers = create_string_table(string);
    }

    char* driver_entry = strtok(path, "\n");
    while (driver_entry != NULL) {
        if (strlen(driver_entry)==0)
            break;

        char* dd = driver_entry;
        char* driver_id = driver_entry;
        while (*dd != 0) {
            if (*dd == ':') {
                *dd = 0;
                break;
            }
            ++dd;
        }
        char* driver_path = dd+1;

        if (table_set(string, string, drivers, driver_id, driver_path)) {
            // TODO: add kernel shutdown
        }

        driver_entry = strtok(NULL, "\n");
    }
}

void load_from_initramfs(const char* path) {
    ifs_file_t f;
    get_file(path, &f);

    char* contents = malloc(f.entry.num_ent_or_size+1);
    if (contents == NULL) {
        // TODO: kernel panic
        return;
    }
    memcpy(contents, f.file_contents, f.entry.num_ent_or_size);
    contents[f.entry.num_ent_or_size] = 0;
    load_drivers(contents);
}

void load_from_disk(const char* path) {
    // TODO
}
