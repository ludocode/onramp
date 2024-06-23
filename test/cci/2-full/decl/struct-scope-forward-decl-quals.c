// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// this is a forward declaration of a struct at file scope, even though it has
// useless qualifiers
// (TODO this apparently only forward declares at file scope; within a
// function, qualifiers make it not forward declare in GCC. i haven't yet
// figured out a test case where this matters.)
const struct a volatile;

// foo() uses the `struct a` declared at file scope
int foo(struct a* a) {
    return 0;
}

// define `struct a`
struct a {
    int x;
};

int main(void) {
    struct a a;
    return foo(&a); // no problem
}
