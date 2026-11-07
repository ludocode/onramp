// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    double f1 = 1;
    if (f1 != 1) return 1;

    double f2 = 2ll;
    if (f2 != 2) return 2;

    double f3 = 3llu;
    if (f3 != 3) return 3;
}
