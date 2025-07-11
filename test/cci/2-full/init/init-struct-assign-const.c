// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

struct point {
    int x, y;
};

int main(void) {
    const struct point p = {1, 2};
    struct point q = p; // onramp parses this as a cast from const to non-const
    if (q.x != 1) return 1;
    if (q.y != 2) return 2;
}
