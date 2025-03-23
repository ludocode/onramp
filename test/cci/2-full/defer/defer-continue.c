// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    while (x++ == 0) {
        defer x *= 5;
        x += 3;
        continue;
    }
    if (x != 21) return 1;
}
