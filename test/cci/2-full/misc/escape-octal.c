// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // full-length octal escapes
    char x[] = "\000\123\377";
    if (x[0] != 0) return 1;
    if (x[1] != 83) return 2;
    if (x[2] != -1) return 3;

    // short octal sequences
    char y[] = "\0\18\33";
    if (y[0] != 0) return 4;
    if (y[1] != 1) return 5;
    if (y[2] != '8') return 6;
    if (y[3] != 27) return 7;

}
