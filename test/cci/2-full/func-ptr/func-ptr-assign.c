// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int x = 0;

void foo(void) {
    ++x;
}

void (*bar)(void);
void (*baz)(void);

int main(void) {
    baz = bar = foo;
    foo();
    if (x != 1) return 1;
    bar();
    if (x != 2) return 2;
    baz();
    if (x != 3) return 3;
}
