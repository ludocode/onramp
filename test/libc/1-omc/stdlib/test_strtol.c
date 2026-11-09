// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>

#include <errno.h>
#include <limits.h>
#include <string.h>

int main(void) {
    errno = 0;

    // test some base 0 detection
    if (123456 != strtol("123456", NULL, 0)) {return 1;}
    if (0x123 != strtol("0x123", NULL, 0)) {return 2;}
    if (0XABC != strtol("0XABC", NULL, 0)) {return 3;}
    if (420 != strtol("0644", NULL, 0)) {return 4;}
    if (420 != strtol("0644", NULL, 0)) {return 5;}
    // not all libcs support 0b yet, we use this #ifdef to be able to run this
    // test against other libcs to ensure our behaviour matches
    #ifdef __onramp_libc__
    if ('H' != strtol("0b1001000", NULL, 0)) {return 6;}
    if ('i' != strtol("0b1101001", NULL, 0)) {return 7;}
    #endif

    if (INT_MAX != strtol("1111111111111111111111111111111", NULL, 2)) {return 8;}
    if (INT_MIN != strtol("-10000000000000000000000000000000", NULL, 2)) {return 9;}

    // test specific bases
    if (0x1a2b3c != strtol("1a2B3c", NULL, 16)) {return 10;}
    if (54 != strtol("312", NULL, 4)) {return 11;}
    if (2122381809 != strtol("Z3m0A9", NULL, 36)) {return 12;}

    // test leading whitespace and end parameter
    char* end;
    if (-13521 != strtol(" \t\n\v\f\r-13521ABC", &end, 0)) {return 13;}
    if (0 != strcmp(end, "ABC")) {return 14;}
    if (493 != strtol("0755819", &end, 0)) {return 15;}
    if (0 != strcmp(end, "819")) {return 16;}

    if (errno != 0) {return 17;}

    // test some error detection
    if (INT_MAX != strtol("2147483648", NULL, 0)) {return 18;}
    if (errno != ERANGE) {return 19;}
    errno = 0;
    if (INT_MIN != strtol("-2147483649", NULL, 0)) {return 20;}
    if (errno != ERANGE) {return 21;}

    // test end parameter on overflow
    errno = 0;
    if (INT_MAX != strtol("123456789123456789  ABC", &end, 0)) {return 22;}
    if (errno != ERANGE) {return 23;}
    if (0 != strcmp(end, "  ABC")) {return 24;}
    errno = 0;
    if (INT_MIN != strtol("-123456789123456789\tzyx", &end, 0)) {return 25;}
    if (errno != ERANGE) {return 26;}
    if (0 != strcmp(end, "\tzyx")) {return 27;}

    return 0;
}
