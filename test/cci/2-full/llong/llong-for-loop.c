// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    unsigned long long u = 0llu;
    for (; u != 5u; ++u);
    if (u != 5u) return 1;
}
