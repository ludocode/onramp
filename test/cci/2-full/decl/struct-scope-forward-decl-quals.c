// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This is technically *not* a forward declaration of a struct because of the
// qualifiers, but since no type with this tag exists, it declares a struct in
// this scope anyway.
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
