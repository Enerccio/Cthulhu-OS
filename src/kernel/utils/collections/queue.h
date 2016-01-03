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
 * queue.h
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../../commons.h"

typedef struct queue_element queue_element_t;

struct queue_element {
	bool __pool_c;
	void* data;
	queue_element_t* previous;
	queue_element_t* next;
};

typedef struct queue_pool {
	void* pool_data;
	uint32_t max_size;
} queue_pool_t;

typedef struct queue {
	bool is_static;
	queue_element_t* first;
	queue_element_t* last;
	uint32_t size;
	queue_pool_t queue_pool;
} queue_t;

typedef bool (*find_func_t) (void* data, void* element);

queue_t* create_queue();

void* queue_pop(queue_t* queue);

void* queue_peek(queue_t* queue);

bool queue_has_elements(queue_t* queue);

bool queue_push(queue_t* queue, void* data);

uint32_t queue_size(queue_t* queue);

void free_queue(queue_t* queue);

queue_t* create_queue_static(uint32_t queue_max_size);

void queue_remove(void* element, queue_t* queue);

void* queue_find_by_func(void* data, find_func_t func, queue_t* queue);
