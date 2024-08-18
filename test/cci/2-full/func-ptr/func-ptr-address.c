// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (main != &main) return 1;
    int (*p)(void) = main;
    if (p != main) return 2;
    if (p != &main) return 3;
    if (p != p) return 4;
    if (*&p != p) return 5;
    if (sizeof(p) != sizeof(int (*)(void))) return 6;
    if (sizeof(&main) != sizeof(int (*)(void))) return 7;
}
