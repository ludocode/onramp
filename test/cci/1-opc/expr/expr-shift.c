// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((5u >> 2) != 1u) return 1;
    if ((5u >> 0) != 5u) return 2;
    if ((5 >> 2) != 1) return 3;
    if ((5 >> 0) != 5) return 4;

    if ((5u << 2) != 20u) return 5;
    if ((5u << 0) != 5u) return 6;
    if ((5 << 2) != 20) return 7;
    if ((5 << 0) != 5) return 8;
}
