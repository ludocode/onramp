// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 1;
    switch (1) {
        defer x = 2;
        case 1: // error: case crosses defer
    }
    return x;
}
