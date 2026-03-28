/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#include <errno.h>
#include <stdio.h>

#include "internal.h"

int errno;

#ifndef __onramp_cci_opc__
// The order here must match the numbering in <errno.h>
// (TODO: once we implement them in cci/2 we could use array designators.)
const char* const sys_errlist[] = {
    "(no error)",
    "An argument is outside the domain of the function. (EDOM)",
    "Invalid character encoding sequence. (EILSEQ)",
    "An argument is out of range. (ERANGE)",
    "An argument is invalid. (EINVAL)",
    "Out of memory. (ENOMEM)",
    "The function was interrupted. (EINTR)",
    "This is a directory. (EISDIR)",
    "This is not a directory. (ENOTDIR)",
    "Too many open files. (ENFILE)",
    "Access denied. (EACCES)",
    "The given file descriptor is invalid. (EBADF)",
    "An I/O error occurred. (EIO)",
    "Seeking is not supported on this file. (ESPIPE)",
    "The value is too large. (EOVERFLOW)",
    "This operation is not supported. (ENOTSUP)",
    "Data is not available yet; try later. (EAGAIN/EWOULDBLOCK)",
    "The other end of the stream is closed. (EPIPE)",
    "No such file. (ENOENT)",
    "The filename is too long. (ENAMETOOLONG)",
};

int sys_nerr = sizeof(sys_errlist) / sizeof(sys_errlist[0]);
#endif

void perror(const char* s) {
    if (s && *s) {
        fputs(s, stderr);
        fputs(": ", stderr);
    }

    #ifndef __onramp_cci_opc__
    // The user can set errno. We make sure it's in bounds.
    if (errno < 0 || errno >= sys_nerr) {
        fputs("(unrecognized errno)", stderr);
    } else {
        fputs(sys_errlist[errno], stderr);
    }
    #endif

    #ifdef __onramp_cci_opc__
    // We don't have strings. Just print the error number.
    char buffer[16];
    fwrite(buffer, 1, __utod(errno, buffer), stderr);
    #endif

    fputc('\n', stderr);
}
