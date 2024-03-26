// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // Only the sign of the left hand side of a shift matters.
    if ((unsigned)(-1) >> (unsigned)16 != 0xFFFFu) {return 1;}
    if ((  signed)(-1) >> (unsigned)16 !=      -1) {return 1;}
    if ((unsigned)(-1) >> (  signed)16 != 0xFFFFu) {return 1;}
    if ((  signed)(-1) >> (  signed)16 !=      -1) {return 1;}

    return 0;
}
