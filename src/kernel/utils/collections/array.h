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
 * array.h
 *  Created on: Dec 24, 2015
 *      Author: Peter Vanusanik
 *  Contents: array implementation
 */

#pragma once

#include "../../commons.h"
#include "../random.h"
#include <stdlib.h>
#include <string.h>

/** Number of elements in starting size */
#define ARRAY_STARTING_SIZE 32

typedef bool (* array_search_predicate_t) (void* element, void* passed_data);

typedef struct {
	void**   data;
	uint32_t size;
	uint32_t data_size;
	uint32_t starting_size;
} array_t;

/**
 * Creates new array with default allocation size
 */
array_t* create_array();

/**
 * Creates new array with specified number of elements preallocated.
 */
array_t* create_array_spec(uint32_t starting_element_size);

/**
 * Pushes data to array.
 */
uint32_t array_push_data(array_t* array, void* data);

/**
 * Returns index to data, if the data is in the array.
 * Returns -1 if data is not found.
 */
int32_t array_find_data(array_t* array, void* data);

/**
 * Inserts data at position (enlarging if necessary).
 */
void array_insert_at(array_t* array, uint32_t pos, void* data);

/**
 * Returns data at position in array. Returns NULL if no data can be found.
 */
void* array_get_at(array_t* array, uint32_t position);

/**
 * Sets data at position, overwriting any data.
 * If position is not within array, does nothing.
 */
void array_set_at(array_t* array, uint32_t position, void* data);

/**
 * Removes item at position in array, moving elements as necessary.
 */
void array_remove_at(array_t* array, uint32_t position);

/**
 * Returns number of elements in this array
 */
uint32_t array_get_size(array_t* array);

/**
 * Frees the array's array and array.
 */
void destroy_array(array_t* array);

/**
 * Finds the data that matches predicate. Predicate is called with
 * data element and data provided and returns true or false.
 */
void* array_find_by_pred(array_t* array, array_search_predicate_t predicate, void* data);

// defined as inline
/** Cleans the array, returning number of elements to 0. Keeps the buffer active */
#define array_clean(array) array->size = 0

/*
 * Gets random element from array
 */
void* array_get_random(array_t* array, rg_t* rg);
