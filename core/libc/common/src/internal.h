/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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

#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

#include <__onramp/__fatal.h>
#include <__onramp/__size_t.h>
#include <assert.h>
#include <stdint.h>

#ifdef __onramp_cpp_omc__
    #define libc_assert assert
#endif
#ifndef __onramp_cpp_omc__
    #define libc_assert(expression) \
        ((expression) ? ((void)0) : \
            __assert_fail(#expression, __FILE__, __LINE__, __func__))
#endif

void __call_atexit(void);
void __call_at_quick_exit(void);

// setup and teardown functions
void __time_setup(void);
void __stdio_setup(void);
void __stdio_teardown(void);

#ifndef __onramp_cci_omc__
// u32 to decimal text. returns length, does not null-terminate
size_t __utod(uint32_t value, char* output);
#endif

#ifndef __onramp_cci_omc__
/**
 * Reads a directory entry from the given file descriptor.
 *
 * Returns 1 on success; __name contains a null-terminated filename.
 *
 * Returns 0 on end of directory; __name is not set.
 *
 * Returns -1 and sets errno on error; __name contains garbage and must not be
 * used.
 *
 * - EBADF: No such file descriptor or not a directory.
 * - EOVERFLOW: File name of entry is too long. (You may continue reading other
 *              entries.)
 */
int __dirent(int __fd, char __name[256]);
#endif

#endif
