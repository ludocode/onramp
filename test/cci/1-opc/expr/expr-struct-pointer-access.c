// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int x;
    int y;
} foo;

int main(void) {
    struct foo* pfoo = &foo;
    pfoo->x = 2;
    pfoo->y = 3;

    if (pfoo->x != 2) {
        return 1;
    }
    if (pfoo->y != 3) {
        return 1;
    }

    if (*(int*)&foo != 2) {
        return 1;
    }
    if (*((int*)&foo + 1) != 3) {
        return 1;
    }

    return 0;
}
