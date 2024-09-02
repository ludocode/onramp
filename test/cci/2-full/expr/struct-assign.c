// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct P {
    int x, y;
};

int main(void) {
    struct P a = {2, 3};
    struct P b;
    b = a;
    if (b.x + b.y != 5) return 1;
}
