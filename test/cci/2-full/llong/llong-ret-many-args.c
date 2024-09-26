// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// test of call convention with indirect return and too many args to pass in registers
unsigned long long foo(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e) {
    return a + 2*b + 3*c + 4*d + 5*e;
}

int main(void) {
    if (143 != foo(3, 5, 7, 11, 13)) return 1;
}
