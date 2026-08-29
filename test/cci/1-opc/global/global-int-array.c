// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This may be declared as `extern int x[]` in some other translation unit; its
// size is unknown, and so it is unknown whether the compiler will statically
// allocate storage for it. Therefore, in the final Onramp ABI, the symbol `x`
// is actually a pointer to the storage, not the storage itself. Since this
// definition has no initializer, the compiler and libc can allocate it with
// malloc() on startup, simulating bss.
int x[3];

int foo(void) {
    int* px = x;
    return px[0] + px[1] + px[2];
}

int main(void) {
    x[0] = 2;
    x[1] = 3;
    x[2] = 4;
    return 9 - foo();
}
