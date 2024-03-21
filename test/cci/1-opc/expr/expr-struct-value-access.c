// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct {
    int x;
    int y;
} foo;

int main(void) {
    foo.x = 2;
    foo.y = 3;

    if ((*(&foo)).x != 2) {
        return 1;
    }
    if ((*(&foo)).y != 3) {
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
