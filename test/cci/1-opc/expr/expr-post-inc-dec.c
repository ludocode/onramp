// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 5;
    if (x++ != 5) {return 1;}
    if (x != 6) {return 2;}
    if (x-- != 6) {return 3;}
    if (x != 5) {return 4;}
    return 0;
}
