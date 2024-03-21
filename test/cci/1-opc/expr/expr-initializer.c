// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 5 - 3;
    if (x != 2) {return 1;}

    int y = 5;
    int z = y - y;
    if (z != 0) {return 2;}

    return 0;
}
