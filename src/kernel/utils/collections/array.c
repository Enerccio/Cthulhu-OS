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
 * array.c
 *  Created on: Dec 24, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "array.h"

/**
 * Creates new array with default allocation size
 */
array_t* create_array() {
    return create_array_spec(ARRAY_STARTING_SIZE);
}

/**
 * Creates new array with specified number of elements preallocated.
 */
array_t* create_array_spec(uint32_t starting_element_size) {
    array_t* array = (array_t*) malloc(sizeof(array_t));
    if (array == NULL)
        return NULL;
    array->size = 0;
    array->data_size = starting_element_size * sizeof(void*);
    array->data = (void**) malloc(array->data_size);
    if (array->data == NULL) {
        free(array);
        return NULL;
    }
    array->starting_size = starting_element_size;
    return array;
}

/**
 * Pushes data to array.
 */
uint32_t array_push_data(array_t* array, void* data) {
    if (array->size == array->data_size/sizeof(void*)) {
        array->data = (void**) realloc(array->data, array->data_size + (array->starting_size * sizeof(void*)));
        array->data_size += array->starting_size * sizeof(void*);
    }

    array->data[array->size++] = data;
    return array->size-1;
}

/**
 * Returns index to data, if the data is in the array.
 * Returns -1 if data is not found.
 */
int32_t array_find_data(array_t* array, void* data) {
    uint32_t iterator = 0;
    for (; iterator<array->size; iterator++)
        if (array_get_at(array, iterator) == data)
            return iterator;
    return -1;
}

/**
 * Inserts data at position (enlarging if necessary).
 */
void array_insert_at(array_t* array, uint32_t position, void* data) {
    if (position == array->size-1) {
        array_push_data(array, data);
        return;
    }

    if (position > array->size-1)
        return;

    if (array->size == 0) {
        if (position == 0) {
            array_push_data(array, data);
            return;
        } else
            return;
    }

    uint32_t it = position;
    void* prev = data;
    while (it <= array->size-1) {
        void* tmp = prev;
        prev = array->data[it];
        array->data[it] = tmp;
        ++it;
    }

    array_push_data(array, prev); // to ensure that we only have one code that will enlarge array
}

/**
 * Returns data at position in array. Returns NULL if no data can be found.
 */
void* array_get_at(array_t* array, uint32_t position) {
    if (position > array->size-1)
        return 0;

    return array->data[position];
}

/**
 * Sets data at position, overwriting any data.
 * If position is not within array, does nothing.
 */
void array_set_at(array_t* array, uint32_t position, void* data) {
    if (position < array->size-1)
        array->data[position] = data;
}

/**
 * Removes item at position in array, moving elements as necessary.
 */
void array_remove_at(array_t* array, uint32_t position) {
    if (position == array->size-1) {
        --array->size;
        return;
    }

    int32_t p = ((int32_t)position)-1;
    while ((++p)<(int32_t)array->size-1)
        array->data[p] = array->data[p+1];
    --array->size;
}

/**
 * Returns number of elements in this array
 */
uint32_t array_get_size(array_t* array) {
    return array->size;
}

/**
 * Frees the array's array and array.
 */
void destroy_array(array_t* array) {
    free(array->data);
    free(array);
}

/**
 * Finds the data that matches predicate. Predicate is called with
 * data element and data provided and returns true or false.
 */
void* array_find_by_pred(array_t* array, search_predicate_t predicate, void* data) {
    uint32_t i = 0;

    for (; i<array->size; i++)
        if (predicate(array->data[i], data) == true)
            return array->data[i];

    return 0;
}

void* array_get_random(array_t* array, rg_t* rg) {
    if (array->size == 0)
        return NULL;
    ruint_t idx = rg_next_uint_l(rg, array->size);
    return array->data[idx];
}
