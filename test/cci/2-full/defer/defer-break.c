// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    while (1) {
        defer x *= 5;
        x += 3;
        break;
    }
    if (x != 15) return 1;
}
