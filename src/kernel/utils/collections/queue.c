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
 * queue.c
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "queue.h"

queue_t* create_queue() {
	queue_t* queue = (queue_t*) malloc(sizeof(queue_t));
	queue->first = 0;
	queue->last = 0;
	queue->size = 0;
	queue->is_static = false;
	return queue;
}

queue_t* create_queue_static(uint32_t max_queue) {
	queue_t* queue = (queue_t*) malloc(sizeof(queue_t));
	queue->first = 0;
	queue->last = 0;
	queue->size = 0;
	queue->is_static = true;
	queue->queue_pool.max_size = max_queue;
	queue->queue_pool.pool_data = (void*) malloc(sizeof(queue_element_t) * max_queue);
	memset(queue->queue_pool.pool_data, 0, sizeof(queue_element_t) * max_queue);
	return queue;
}

void* queue_pop(queue_t* queue) {
	queue_element_t* queue_head = queue->first;
	if (queue_head == 0)
		return 0;

	if (queue->size == 1) {
		queue->first = 0;
		queue->last = 0;
		queue->size = 0;
		void* data = queue_head->data;
		if (queue->is_static == true)
			queue_head->__pool_c = 0; // ready for scrapping into the pool
		else
			free(queue_head);
		return data;
	}

	void* data = queue_head->data;
	queue->first = queue_head->next;

	if (queue->is_static == true)
		queue_head->__pool_c = 0;
	else
		free(queue_head);

	queue->size--;
	return data;
}

void* queue_peek(queue_t* queue) {
	queue_element_t* queue_head = queue->first;
	if (queue_head == 0)
		return 0;

	return queue_head->data;
}

bool queue_has_elements(queue_t* queue) {
	return queue_size(queue) > 0;
}

static queue_element_t* get_free_segment(queue_t* queue) {
	uintptr_t block = (uintptr_t) queue->queue_pool.pool_data;
	while (block < ((uintptr_t) queue->queue_pool.pool_data) +
			(queue->queue_pool.max_size * sizeof(queue_element_t))) {
		queue_element_t* el = (queue_element_t*) block;
		if (el->__pool_c == 0) {// it is a free block, yay
			memset(el, 0, sizeof(queue_element_t)); // purge old data
			el->__pool_c = true; // now it is used
			return el;
		}
		block += sizeof(queue_element_t);
	}
	// we have exhausted all the memory, return instantly
	return 0;
}

bool queue_push(queue_t* queue, void* data) {
	queue_element_t* element;
	if (queue->is_static == true)
		element = get_free_segment(queue);
	else
		element = (queue_element_t*) malloc(sizeof(queue_element_t));

	if (element == 0)
		return true;

	element->data = data;
	element->next = 0;
	element->previous = 0;

	if (queue->first == 0) {
		queue->first = queue->last = element;
		queue->size = 1;
	} else {
		queue->last->next = element;
		element->previous = queue->last;
		queue->last = element;
		queue->size++;
	}

	return false;
}

uint32_t queue_size(queue_t* queue) {
	return queue->size;
}

void free_queue(queue_t* queue) {
	if (queue->is_static == true) {
		free(queue);
		free(queue->queue_pool.pool_data);
	} else {
		queue_element_t* el = queue->first;
		while (el != 0) {
			queue_element_t* tmp = el->next;
			free(el);
			el = tmp;
		}
	}
}

void queue_remove(void* element, queue_t* queue) {
	if (queue->size == 0)
		return;

	queue_element_t* e = queue->first;
	while (e != 0 && e->data != element)
		e = e->next;

	if (e) {
		e->previous->next = e->next;
		if (e->next)
			e->next->previous = e->previous;
		if (queue->is_static == true)
			e->__pool_c = 0;
		else
			free(e);
	}
}

void* queue_find_by_func(void* data, find_func_t func, queue_t* queue) {
	if (queue->size == 0)
		return 0;

	queue_element_t* e = queue->first;
	while (e != 0 && func(e->data, data) == false)
		e = e->next;

	if (e)
		return e->data;
	else
		return 0;
}
