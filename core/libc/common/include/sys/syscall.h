/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_SYS_SYSCALL_H_INCLUDED
#define __ONRAMP_LIBC_SYS_SYSCALL_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

/*
 * Onramp doesn't currently have public SYS_* constants but the libc does
 * define __SYS_* constants as part of its internal API. There also is not a
 * UNIX-style syscall() wrapper; instead there is an individual __sys_*()
 * function for each syscall as well as a __syscall_is_supported() function.
 *
 * These internal symbols are accessible through this header and they can be
 * used by programs but they may change at any time. See the below header for
 * details.
 *
 * It would be easy to write a proper UNIX-style syscall() function in assembly
 * but this is not done for two reasons: first, there has been no need, and
 * second, it would require committing to a permanent public API for syscalls
 * in the Onramp libc.
 */

#include <__onramp/__syscalls.h>

#endif
