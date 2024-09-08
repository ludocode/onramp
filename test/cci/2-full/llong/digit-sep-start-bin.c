// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    0b'111; // ERROR, digit separator is not allowed after a 0b/0x prefix
}
