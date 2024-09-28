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

#include <time.h>

#include <errno.h>

#include "syscalls.h"

// We store these timestamps in order to implement clock() and CLOCK_MONOTONIC.
static struct timespec __start_time; // real time at start of process
static struct timespec __last_time;  // real time at last query of monotonic clock
static struct timespec __mono_time;  // mono time at last query of monotonic clock

// Called on process start before main() and before user constructors.
void __time_setup(void) {
    int e = __sys_time((unsigned*)&__start_time);
    if (e != 0) {
        // TODO handle the case of VM not giving us time
    }
    __last_time = __start_time;
}

// Adds two timespecs.
static void timespec_add(struct timespec* out, struct timespec* left, struct timespec* right) {
    out->tv_sec = left->tv_sec + right->tv_sec;
    out->tv_nsec = left->tv_nsec + right->tv_nsec;
    if (out->tv_nsec >= 1000000000) {
        out->tv_nsec -= 1000000000;
        out->tv_sec += 1;
    }
}

// Subtracts two timespecs.
static void timespec_subtract(struct timespec* out, struct timespec* left, struct timespec* right) {
    out->tv_sec = left->tv_sec - right->tv_sec;
    if (left->tv_nsec < right->tv_nsec) {
        out->tv_nsec = 1000000000 - right->tv_nsec + left->tv_nsec;
        --out->tv_sec;
    } else {
        out->tv_nsec = left->tv_nsec - right->tv_nsec;
    }
}

static int clock_realtime(struct timespec* out) {
    int e = __sys_time((unsigned*)out);
    if (e != 0) {
        // TODO set errno properly based on syscall return value
        errno = ENOTSUP;
        return -1;
    }

    //fprintf(stderr, "__sys_time returned %i value %x %x %x\n",e,((unsigned*)out)[0], ((unsigned*)out)[1], ((unsigned*)out)[2]);
    return 0;
}

static int clock_monotonic(struct timespec* out) {
    struct timespec now;
    int e = clock_realtime(&now);
    if (e != 0) {
        // Let clock_realtime() set errno for us
        return -1;
    }

    // Get the delta time since the last call
    struct timespec delta;
    timespec_subtract(&delta, &now, &__last_time);

    // Increment the monotonic time only if it hasn't gone backwards
    if (delta.tv_sec >= 0) {
        timespec_add(&__mono_time, &__mono_time, &delta);
    }

    __last_time = now;
    *out = __mono_time;
    return 0;
}

int clock_gettime(clockid_t clock, struct timespec* timespec) {
    switch (clock) {
        case CLOCK_MONOTONIC: return clock_monotonic(timespec);
        case CLOCK_REALTIME: return clock_realtime(timespec);
        default: break;
    }

    errno = ENOTSUP;
    return -1;
}

int timespec_get(struct timespec* timespec, int base) {
    if (base != TIME_UTC) {
        errno = ENOTSUP;
        return 0;
    }
    if (0 != clock_realtime(timespec))
        return 0;
    return base;
}

time_t time(time_t* /*nullable*/ out_time) {
    struct timespec timespec;
    if (0 != clock_gettime(CLOCK_REALTIME, &timespec)) {
        // clock_gettime() would have set errno, although not to one of the
        // allowed values from time(). This call isn't really supposed to be
        // able to fail; it might make more sense to call __fatal() here.
        return ((time_t)-1);
    }
    if (out_time)
        *out_time = timespec.tv_sec;
    return timespec.tv_sec;
}

clock_t clock(void) {
    struct timespec timespec;
    if (0 != clock_gettime(CLOCK_MONOTONIC, &timespec)) {
        return (clock_t)-1;
    }
    return timespec.tv_sec * 1000000 + timespec.tv_nsec / 1000;
}

int gettimeofday(struct timeval* timeval, struct timezone* ignored) {
    (void)ignored;

    struct timespec timespec;
    if (0 != clock_realtime(&timespec))
        return -1;

    timeval->tv_sec = timespec.tv_sec;
    timeval->tv_usec = timespec.tv_nsec / 1000;
    return 0;
}
