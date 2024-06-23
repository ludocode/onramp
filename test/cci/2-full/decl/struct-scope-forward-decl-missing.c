// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// missing forward declaration `struct a` at file scope

// this declares `struct a` only within the scope of the function
void foo(struct a* a) {
}

// this declares a different `struct a`
struct a {
    int x;
};

int main(void) {
    struct a a;
    foo(&a); // ERROR, types don't match
}
