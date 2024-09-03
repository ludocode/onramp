// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    static int x[] = {1, 2, 3};
    if (sizeof(x) != 3 * sizeof(int)) return 1;
    if (6 != x[0] + x[1] + x[2]) return 2;
}
