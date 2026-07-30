// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct s {
    int x;
    char y[6];
    long long z;
};

int main(void) {
    // When sizeof is followed by parens, it may contain a type, or it may
    // *begin* a unary expression that starts with a parenthesized expression.
    // In other words, if it's an expression, the parens don't necessarily
    // contain the whole expression.
    if (sizeof ((struct s*)0)->x != 4) return 1;
    if (sizeof ((struct s*)0)->y != 6) return 2;
    if (sizeof ((struct s*)0)->z != 8) return 3;
}
