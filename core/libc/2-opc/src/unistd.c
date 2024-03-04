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

#include <unistd.h>

#include "internal.h"

ssize_t write(int fd, const void* buffer, size_t count) {
    #if defined(__onramp__)
        return __builtin_onramp_vm_write(fd, buffer, count);
    #elif defined(__linux__) && defined(__i386__)
        ssize_t ret;
        __asm__(
                "movl $4, %%eax\n"  // 4 == SYS_write on i386
                "movl %[fd], %%ebx\n"
                "movl %[buffer], %%ecx\n"
                "movl %[count], %%edx\n"
                "int $0x80\n"
                "movl %%eax, %[ret]\n"
                : // outputs
                [ret] "=r" (ret)
                : // inputs
                [fd] "rm" (fd),
                [buffer] "rm" (buffer),
                [count] "rm" (count)
                : // registers clobbered
                "eax",
                "ebx",
                "ecx",
                "edx"
        );
        return ret;
    #endif
}
