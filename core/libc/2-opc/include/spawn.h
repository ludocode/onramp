/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_SPAWN_H_INCLUDED
#define __ONRAMP_LIBC_SPAWN_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

int posix_spawn(void); //TODO

/**
 * Runs an Onramp executable as a child process within this VM.
 *
 * The current process blocks until the child process is complete. The child's
 * exit code is returned as an unsigned byte (0-255).
 *
 * If any FILEs are associated with the given file descriptors, they will be
 * flushed before the child process is spawned. Use fileno() to get the file
 * descriptor of a FILE. Pass -1 to use the defaults.
 *
 * If an error occurs, -1 is returned and errno is set. Potential errors are:
 *
 * - ENOENT: Executable does not exist
 * - ENOMEM: Out of memory
 * - ENFILE: Too many open files
 *
 * The largest block of unused heap will be used as the child process's
 * memory. This must be large enough to store the process's image, stack and
 * heap.
 */
int __onramp_spawn(
        const char* executable,
        const char* working_directory,
        const char** arguments,
        const char** environment,
        int stdin_fd,
        int stdout_fd,
        int stderr_fd);

/**
 * Runs an Onramp executable as a child process within this VM.
 *
 * This is the same as __onramp_spawn() except the working directory and
 * standard file descriptors are those of the current process.
 */
int __onramp_spawn_basic(
        const char* executable,
        const char** arguments,
        const char** environment);

#endif
