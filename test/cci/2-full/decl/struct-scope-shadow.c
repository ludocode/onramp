// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct a {
    int x;
};

int main(void) {
    struct a a;
    a.x = 4;

    struct a {  // shadow `struct a`
        int y;
    };
    struct a b;
    b.y = 4;

    if (a.x != b.y)
        return 1;
}
