// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int a;
    struct bar {
        int x;
        int y;
    } b;
    int c;
};

int main(void) {
    if (sizeof(struct foo) != 16) {
        return 1;
    }

    struct foo f;
    if (sizeof(f) != 16) {
        return 1;
    }

    return 0;
}
