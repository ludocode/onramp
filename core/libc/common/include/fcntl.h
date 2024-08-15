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

#ifndef __ONRAMP_LIBC_FCNTL_H_INCLUDED
#define __ONRAMP_LIBC_FCNTL_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__mode_t.h>

// one of these is required
#define O_RDONLY     0x1
#define O_WRONLY     0x2
#define O_RDWR       0x4

// optional flags
#define O_APPEND     0x8
#define O_CREAT      0x10
#define O_DIRECTORY  0x20
#define O_TRUNC      0x40

// ignored flags
#define O_ASYNC      0
#define O_DSYNC      0
#define O_SYNC       0
#define O_CLOEXEC    0
#define O_DIRECT     0
#define O_NOATIME    0
#define O_NONBLOCK   0

int open(const char* __path, int __flags, ...);
int creat(const char* __path, mode_t __mode);

#endif
