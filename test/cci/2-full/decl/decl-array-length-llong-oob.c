// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

//% FAIL
//% NON_STANDARD

int x[0x100000000LL]; // ERROR: too large (on 32-bit platforms)

int main(void) {}
