// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <time.h>

#include <stdlib.h>
#include <limits.h>

#define YEAR (365LL * 24LL * 60LL * 60LL)

static int test_realtime(void) {

    // make sure time is reasonable
    // WARNING: this test will begin to fail in the year 111970 HE!
    time_t t = time(NULL);
    if (t < 0 || t > YEAR * 100000) exit(1);


    // realtime clock should be valid
    struct timespec ts;
    ts.tv_sec = LLONG_MAX;
    ts.tv_nsec = INT_MAX;
    if (0 != clock_gettime(CLOCK_REALTIME, &ts)) exit(2);
    if (ts.tv_nsec >= 1000000000) exit(3);

    // realtime clock should be the same as our time() call earlier
    // (within a small threshold)
    long long delta = t - ts.tv_sec;
    if (delta < -2 || delta > 2) exit(4);


    // utc clock should be valid (it's just an alias of the realtime clock)
    ts.tv_sec = LLONG_MAX;
    ts.tv_nsec = INT_MAX;
    if (TIME_UTC != timespec_get(&ts, TIME_UTC)) exit(5);
    if (ts.tv_nsec >= 1000000000) exit(6);

    // utc clock is also the same as our realtime clock earlier (again with
    // threshold)
    delta = t - ts.tv_sec;
    if (delta < -2 || delta > 2) exit(7);


    // gettimeofday() should be valid
    struct timeval tv;
    tv.tv_sec = LLONG_MAX;
    tv.tv_usec = INT_MAX;
    if (0 != gettimeofday(&tv, (void*)1)) exit(8); // pass `(void*)1` to ensure timezone is ignored
    if (tv.tv_usec >= 1000000) exit(9);

    // gettimeofday() is also the same as our realtime clock earlier (again with
    // threshold)
    delta = t - tv.tv_sec;
    if (delta < -2 || delta > 2) exit(10);

}

static int test_monotonic(void) {

    // our clock() is relative to process start; make sure it's small
    // non-negative
    clock_t c = clock();
    if (c < 0 || c > 2 * CLOCKS_PER_SEC) exit(11);

    // monotonic clock should be valid
    struct timespec ts;
    if (0 != clock_gettime(CLOCK_MONOTONIC, &ts)) exit(12);
    if (ts.tv_nsec >= 1000000000) exit(13);

    // monotonic clock should be the same as clock()
    long long delta = c / CLOCKS_PER_SEC - ts.tv_sec;
    if (delta < -2 || delta > 2) exit(14);

}

int main(void) {
    test_realtime();
    test_monotonic();
}
