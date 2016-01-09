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
 * devsys.h
 *  Created on: Jan 6, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#define DS_ERROR_SUCCESS        0
#define DS_ERROR_NOT_IMPLEMENTED -1
#define DS_ERROR_NOT_ALLOWED    -2
#define DS_ERROR_NOT_AUTHORIZED -3
#define DS_ERROR_MODE_UNSUPPORTED -4

#define DEV_SYS_FRAMEBUFFER_GET_WIDTH   1
#define DEV_SYS_FRAMEBUFFER_GET_HEIGHT  2
#define DEV_SYS_IVFS_GET_PATH_ELEMENT   3
