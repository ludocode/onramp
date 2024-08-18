// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    char x[1];
    *x = 4;
    if (x != &x) {
        return 1;
    }
    if (*x != 4) {
        return 2;
    }

    // This is non-standard. The opC type system cannot support pointers to
    // arrays so we've implemented the address-of operator on an array as
    // yielding a pointer to its first element instead of a pointer to the
    // array. In full C, `*&x == x` and `**&x == 4`, but in opC, `*&x == 4`.
    // See cci/2 test `expr/expr-array-conversion.c` for the full C test.
    if (*&x != 4) {
        return 3;
    }

    // TODO it might make more sense to just forbid taking the address of an
    // array in opC. We're not really a proper subset of C if there is code
    // that behaves differently under opC compared to C. We just have to make
    // sure we don't use address-of on arrays anywhere in Onramp and instead
    // always rely on the implicit pointer-to-array conversion.

    return 0;
}
