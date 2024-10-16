// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int a;
    char b;
    short c[];
};

int main(void) {
    if (sizeof(struct foo) != 8) {
        return 1;
    }

    struct foo f;
    if (sizeof(f) != 8) {
        return 2;
    }

    short* s = f.c;
    if ((char*)s - (char*)&f != 6)
        return 3;

    return 0;
}
