// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {

    // Both types are smaller than int, and all values of both types fit in
    // signed int, so both types are promoted to signed int.
    unsigned short us = 1;
    signed short ss = -2;
    if (us + ss >= 0) {return 1;} // signed comparison
    if (us + ss != -1) {return 1;} // signed comparison

    // Both types are of width int, and neither type can represent all values
    // of the other, so both types are promoted to unsigned. (This happens in
    // both the addition and the comparison, so the result of the addition is
    // UINT_MAX and both comparisons are unsigned.)
    unsigned int ui = 1;
    signed int si = -2;
    if (ui + si <= 0) {return 2;} // unsigned comparison
    if (ui + si != -1) {return 2;} // unsigned comparison, -1 converted to UINT_MAX
    if (ui + si != UINT_MAX) {return 2;} // unsigned comparison

    return 0;
}
