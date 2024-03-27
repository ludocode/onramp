// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int c;
    union {
        int x;
        struct {
            int y;
        };
    };
    struct {
        union {
            int a;
        };
        int b;
    };
} foo;

int main(void) {

    // all members are accessible through the base record
    foo.x = 1;
    foo.y = 2;
    foo.a = 3;
    foo.b = 4;
    foo.c = 5;

    // union members have the last assigned value
    if (foo.x != 2) return 1;
    if (foo.y != 2) return 2;

    // struct members have distinct values
    if (foo.a != 3) return 3;
    if (foo.b != 4) return 4;
    if (foo.c != 5) return 5;

    return 0;
}
