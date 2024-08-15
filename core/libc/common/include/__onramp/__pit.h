/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ONRAMP_LIBC_ONRAMP_PIT_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_PIT_H_INCLUDED

/**
 * The process info table.
 */
extern unsigned* __process_info_table;

#define __ONRAMP_PIT_VERSION 0
#define __ONRAMP_PIT_BREAK 1
#define __ONRAMP_PIT_EXIT 2
#define __ONRAMP_PIT_INPUT 3
#define __ONRAMP_PIT_OUTPUT 4
#define __ONRAMP_PIT_ERROR 5
#define __ONRAMP_PIT_ARGS 6
#define __ONRAMP_PIT_ENVIRON 7
#define __ONRAMP_PIT_WORKDIR 8

#endif
