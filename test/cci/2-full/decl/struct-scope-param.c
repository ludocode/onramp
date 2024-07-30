// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// The first `struct x` is visible only within the scope of the function
int foo(struct x {int x;}* x, void (*y)(struct x {int y;})) {
    if (x == 0) {
        struct x z;
        z.x = 2;
        return foo(&z, 0);
    }
    return x->x;
}

int main(void) {
    return 2 - foo(0, 0);
}
