// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    struct {
        int a, b, c;
    } x = {1, 2, 3};
    return 6 - x.a - x.b - x.c;
}
