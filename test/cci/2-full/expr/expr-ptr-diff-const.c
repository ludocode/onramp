// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char x[6];

    char*       a = &x[0];
    char* const b = &x[5];
    if (5 != (b - a)) return 1;

    char      * c = &x[0];
    char const* d = &x[5];
    if (5 != (d - c)) return 2;
}
