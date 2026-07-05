// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if ((unsigned long long)+1ll != 1ull) return 1;
    if ((unsigned long long)-1ll != 0xffffffffffffffffull) return 2;
    if ((unsigned long long)~1ll != 0xfffffffffffffffeull) return 3;
    if (!1ll) return 4;
    if (!0x8000000000000000ll) return 5;
    if (0ll) return 6;
}
