// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    struct s {
        int x, y, z;
    } v = {0};
    if (v.x != 0) return 1;
    if (v.y != 0) return 2;
    if (v.z != 0) return 3;
}
