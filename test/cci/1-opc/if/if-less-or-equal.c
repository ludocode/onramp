// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    if (5 < 1) {
        return 1;
    }
    if (5 < -1) {
        return 1;
    }
    if (1 < -5) {
        return 1;
    }
    if (-5 < -10) {
        return 1;
    }
    if ((INT_MIN+1) < INT_MIN) {
        return 1;
    }
    if (INT_MAX < (INT_MAX-1)) {
        return 1;
    }
    if (INT_MAX < INT_MIN) {
        return 1;
    }

    // equal is true
    if (!(1 <= 1)) {
        return 1;
    }
    if (!(INT_MIN <= INT_MIN)) {
        return 1;
    }
    if (!(INT_MAX <= INT_MAX)) {
        return 1;
    }
    return 0;
}
