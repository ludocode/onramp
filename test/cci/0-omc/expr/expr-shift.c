// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((5 >> 2) != 1) { return 1; }
    if ((5 >> 0) != 5) { return 2; }
    if ((5 << 2) != 20) { return 3; }
    if ((5 << 0) != 5) { return 4; }
}
