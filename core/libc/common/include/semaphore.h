/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Haelwenn (lanodan) Monnier
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

#ifndef __ONRAMP_LIBC_SEMAPHORE_H_INCLUDED
#define __ONRAMP_LIBC_SEMAPHORE_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__fatal.h>
#include <__onramp/__timespec.h>
#include <errno.h>

// The below semaphore calls are stubbed out for now because Onramp doesn't
// support threads. If we do end up adding thread support, for cooperative
// threads we'll need to fix sem_wait() and sem_timedwait(), and for preemptive
// threads we'll also need to use a compare-and-swap instruction.

// Note that sem_open(), sem_unlink() and SEM_FAILED are not implemented and
// the shared argument to sem_init() is ignored. Onramp cannot support named
// semaphores or shared semaphores.

typedef struct {
    unsigned int __value;
} sem_t;

static inline int sem_close(sem_t* __sem) {
    // nothing
}

static inline int sem_destroy(sem_t* __sem) {
    // nothing
}

static inline int sem_getvalue(sem_t* restrict __sem, int* restrict __out_value) {
    *__out_value = (int)__sem->__value;
    return 0;
}

static inline int sem_init(sem_t* __sem, int __shared, unsigned __value) {
    __sem->__value = __value;
    return 0;
}

static inline int sem_post(sem_t* __sem) {
    ++__sem->__value;
    return 0;
}

static inline int sem_wait(sem_t* __sem) {
    if (__sem->__value == 0) {
        __fatal("sem_wait() was called on a semaphore with value 0. Onramp does not yet support threads.");
    }
    --__sem->__value;
    return 0;
}

int sem_timedwait(sem_t* restrict __sem, const struct timespec* restrict __timespec) {
    if (__sem->__value == 0) {
        __fatal("sem_timedwait() was called on a semaphore with value 0. Onramp does not yet support threads.");
    }
    --__sem->__value;
    return 0;
}

static inline int sem_trywait(sem_t* __sem) {
    if (__sem->__value == 0) {
        errno = EAGAIN;
        return -1;
    }
    --__sem->__value;
    return 0;
}

#endif /* __ONRAMP_LIBC_SEMAPHORE_H_INCLUDED */
