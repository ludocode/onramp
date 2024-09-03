// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct P {
    int x, y;
} a, b = {2, 3}, *c = &b;

int main(void) {
    a = *c;
    if (a.x != 2) return 1;
    if (a.y != 3) return 2;
}
