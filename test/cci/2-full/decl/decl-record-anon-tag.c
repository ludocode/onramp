// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This tests anonymous struct/union members of struct/union type having tags.
// This is a feature included under `-fms-extensions` and `-fplan9-extensions`
// in GCC-style compilers. The struct can be defined externally or inline.

struct a {
    int x;
    int y;
};

union c {
    struct a;
    struct b {
        int z;
    };
    int w;
};

// struct b should be available at file scope
struct b b;

int main(void) {

    // test that b works
    b.z = 1;
    if (b.z != 1) return 1;

    // test that we can access all fields of c from the root and unions work as
    // expected
    union c c;
    c.x = 1;
    c.y = 2;
    c.z = 3;
    c.w = 4;
    if (c.x != 4) return 1;
    if (c.y != 2) return 2;
    if (c.z != 4) return 3;
    if (c.w != 4) return 4;

    return 0;
}
