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

#ifndef __ONRAMP_LIBC_TIME_H_INCLUDED
#define __ONRAMP_LIBC_TIME_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__size_t.h>
#include <__onramp/__null.h>
#include <__onramp/__useconds_t.h>

/**
 * The number of virtual clock ticks per second of real time.
 *
 * This is fixed at one million, meaning each virtual clock tick is one
 * microsecond.
 */
#define CLOCKS_PER_SEC 1000000

/**
 * The number of virtual clock ticks (microseconds) since the start of the
 * process.
 *
 * The number of clock ticks per second is given by CLOCKS_PER_SEC. Since it is
 * fixed at one million, this is in units of microseconds.
 */
typedef long long clock_t;

/**
 * Returns the number of virtual clock ticks (microseconds) since the start of
 * the process.
 *
 * Divide this by CLOCKS_PER_SEC to get the number of seconds.
 *
 * On Onramp, this is the same as CLOCK_MONOTONIC, except it is in units of
 * microseconds instead of nanoseconds.
 */
clock_t clock(void);



// TODO time_t is C11 or POSIX

/**
 * A UNIX timestamp in seconds.
 *
 * UNIX time is measured as the number of seconds since the start of January 1st, 1970.
 *
 * On Onramp, time_t is always 64 bits.
 */
typedef long long time_t;

typedef int suseconds_t;

/**
 * Gets the current UNIX timestamp in seconds.
 *
 * The current timestamp is returned. If the given pointer is non-null, the
 * timestamp is also written to it.
 */
time_t time(time_t* /*nullable*/ __out_time);



// TODO require C11 (for timespec_get) or POSIX (for clock_gettime)

/**
 * A timestamp in seconds and nanoseconds.
 */
struct timespec {
    time_t tv_sec; //!< number of seconds as an offset from the base of the given clock
    long tv_nsec;  //!< number of additional nanoseconds in range 0 to 999'999'999
};

/**
 * The clock to query, either CLOCK_REALTIME or CLOCK_MONOTIC.
 */
typedef int clockid_t;

/**
 * A real-time clock measuring UNIX time, i.e. time since January 1st, 1970.
 */
#define CLOCK_REALTIME 1

/**
 * A monotonic clock measuring time since an arbitrary point in the past.
 *
 * On Onramp, the reference time is the start of the process. This clock
 * therefore has the same value as clock() except this has greater range and
 * has units of nanoseconds instead of seconds.
 */
#define CLOCK_MONOTONIC 2

/**
 * Queries the current timestamp from the specified clock and writes it to the
 * given timespec.
 *
 * Returns 0 on success. Returns -1 and sets errno on error.
 */
int clock_gettime(clockid_t __clock, struct timespec* __timespec);

/**
 * A time base measuring UNIX time, i.e. time since January 1st, 1970.
 */
#define TIME_UTC 1

/**
 * Queries the current timestamp in the given time base and writes it to the
 * given timespec.
 *
 * The value of base must be TIME_UTC. This is equivalent to a call to
 * clock_gettime() with CLOCK_REALTIME except for the return value.
 *
 * Returns the given base on success and 0 on error.
 */
int timespec_get(struct timespec* __timespec, int __base);



// TODO require _DEFAULT_SOURCE

/**
 * A timestamp in seconds and microseconds.
 *
 * You should use `struct timespec` and `clock_gettime()` instead of this.
 */
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

/**
 * Gets the current UNIX timestamp in seconds and microseconds, writing it to
 * the given timeval pointer.
 *
 * The timezone argument is ignored.
 *
 * You should use `struct timespec` and `clock_gettime()` instead of this.
 */
int gettimeofday(struct timeval* __timeval, struct timezone* __timezone);

#endif
