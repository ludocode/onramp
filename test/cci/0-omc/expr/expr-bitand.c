// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (1 & 2) {
        return 1;
    }
    if (2 & 4) {
        return 2;
    }
    if (!(1 & 3)) {
        return 3;
    }
    if (!(2 & 6)) {
        return 4;
    }
    return 0;
}
