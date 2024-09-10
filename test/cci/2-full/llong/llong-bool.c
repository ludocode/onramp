// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    long long x = 0x100000000ll;

    // the low word is zero
    if ((int)x != 0) return 1;

    // use in a boolean context is true even though the low word is zero
    if (x); else return 2;

    // same with cast to bool
    _Bool b = x;
    if (b); else return 3;
    if (b != 1) return 4;

    // not operator should work as well
    if (!x) return 5;
}
