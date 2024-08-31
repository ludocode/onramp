// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

enum _ {
    X,
    Y,
    Z
};

int main(void) {
    if (5 & X) return 1;
    if ((5 & Y) != 1) return 2;
    if (5 & Z) return 3;
    if ((5 | X) != 5) return 4;
    if ((5 | Z) != 7) return 5;
    if (5 + Y != 6) return 6;
    if (5 / Z != 2) return 7;
}
