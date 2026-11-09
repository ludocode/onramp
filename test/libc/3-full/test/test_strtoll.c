// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>

#include <errno.h>
#include <limits.h>

int main(void) {
    errno = 0;

    // There's a strtol() test already in libc/1 that is run against libc/3. We
    // mostly want to check boundary conditions of all the variants here.

    errno = 0;
    if (LLONG_MAX != strtoll("111111111111111111111111111111", NULL, 0)) return 1;
    if (errno != ERANGE) return 2;

    errno = 0;
    if (LLONG_MIN != strtoll("-111111111111111111111111111111", NULL, 0)) return 3;
    if (errno != ERANGE) return 4;

    errno = 0;
    if (ULLONG_MAX != strtoull("111111111111111111111111111111", NULL, 0)) return 5;
    if (errno != ERANGE) return 6;

    errno = 0;
    if (ULLONG_MAX != strtoull("-111111111111111111111111111111", NULL, 0)) return 7;
    if (errno != ERANGE) return 8;

    errno = 0;

    if (LLONG_MAX != strtoll("9223372036854775807", NULL, 0)) return 9;
    if (errno != 0) return 10;

    if (LLONG_MAX-1 != strtoll("9223372036854775806", NULL, 0)) return 11;
    if (errno != 0) return 12;

    if (LLONG_MIN != strtoll("-9223372036854775808", NULL, 0)) return 13;
    if (errno != 0) return 14;

    if (LLONG_MIN+1 != strtoll("-9223372036854775807", NULL, 0)) return 15;
    if (errno != 0) return 16;

    if (ULLONG_MAX != strtoull("18446744073709551615", NULL, 0)) return 17;
    if (errno != 0) return 18;

    if (ULLONG_MAX-1 != strtoull("18446744073709551614", NULL, 0)) return 19;
    if (errno != 0) return 20;

    if (ULLONG_MAX != strtoull("-1", NULL, 0)) return 21;
    if (errno != 0) return 22;

    if (ULLONG_MAX-1 != strtoull("-2", NULL, 0)) return 23;
    if (errno != 0) return 24;

    if (1 != strtoull("-18446744073709551615", NULL, 0)) return 25;
    if (errno != 0) return 26;

    if (2 != strtoull("-18446744073709551614", NULL, 0)) return 27;
    if (errno != 0) return 28;
}
