// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    int unused[2];
    int x;
} f;

int main(void) {

    // compound assignment on a member by value
    f.x += 1;
    if (f.x != 1) return 1;

    // compound assignment on a member by pointer
    struct foo* p = &f;
    p->x += 1;
    if (p->x != 2) return 2;
}
