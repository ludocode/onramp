// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    union {
        int x;
        struct {
            int y;
            int z;
        };
    };
    struct {
        union {
            int a;
            int b;
        };
        int c;
    };
    int i;
} foo;

int main(void) {

    // all members are accessible through the base record
    foo.x = 1;
    foo.y = 2;
    foo.z = 3;
    foo.a = 4;
    foo.b = 5;
    foo.c = 6;
    foo.i = 7;

    // union members have the last assigned value
    if (foo.x != 2) return 1;
    if (foo.y != 2) return 2;
    if (foo.a != 5) return 3;
    if (foo.b != 5) return 4;

    // struct members have distinct values
    if (foo.y != 2) return 5;
    if (foo.z != 3) return 6;
    if (foo.b != 5) return 7;
    if (foo.c != 6) return 8;
    if (foo.i != 7) return 8;

    return 0;
}
