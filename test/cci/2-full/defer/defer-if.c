// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    {
        defer
            if (1)
                x *= 3;
        defer
            if (0)
                x += 7;
        x += 2;
    }
    if (x != 6) return 1;
}
