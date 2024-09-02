// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct P {
    int x, y;
};

int foo(struct P a) {
    return a.x + a.y;
}

int main(void) {
    struct P a = {2, 3};
    if (foo(a) != 5) return 1;
}
