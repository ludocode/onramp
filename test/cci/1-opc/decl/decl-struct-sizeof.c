// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int a;
    int b;
    char c;
    char d;
};

int main(void) {
    if (sizeof(struct foo) != 12) return 1;

    struct foo foo;
    if (sizeof(foo) != 12) return 2;
    if ((char*)&foo.b - (char*)&foo.a != 4) return 3;
    if ((char*)&foo.c - (char*)&foo.a != 8) return 5;
    if ((char*)&foo.d - (char*)&foo.a != 9) return 6;

    return 0;
}
