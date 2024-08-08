// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int x;
    int y;
};

struct bar {
    int x[200];
};

int main(void) {
    struct foo foo[3];
    char* a = (char*)foo;
    char* b = (char*)(foo + 1);
    char* c = (char*)(&foo[2]);
    if (b - a != sizeof(struct foo)) return 1;
    if (c - b != sizeof(struct foo)) return 2;
    if (&foo[2] - (foo + 1) != 1) return 3;

    struct bar bar[2];
    if ((char*)(bar + 1) - (char*)bar != 800) return 4;
    if ((bar + 1) - bar != 1) return 5;
}
