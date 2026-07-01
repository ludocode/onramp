// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;

    // basic test of &&
    if (0 && 0) {return 1;}
    if (0 && 1) {return 2;}
    if (1 && 0) {return 3;}
    if (!(1 && 1)) {return 4;}

    // basic test of ||
    if (0 || 0) {return 5;}
    if (!(0 || 1)) {return 6;}
    if (!(1 || 0)) {return 7;}
    if (!(1 || 1)) {return 8;}

    // short-circuiting &&
    if (0 && (x = 1)) {return 9;}
    if (x) {return 10;}
    if (!(1 && (x = 1))) {return 11;}
    if (!x) {return 12;}
    x = 0;

    // short-circuiting ||
    if (!(1 || (x = 1))) {return 13;}
    if (x) {return 14;}
    if (!(0 || (x = 1))) {return 15;}
    if (!x) {return 16;}
    x = 0;

    // short-circuiting chain of &&
    if (1 && 0 && (x = 1)) {return 17;}
    if (x) {return 18;}
    if (0 && (x = 1) && (x = 1)) {return 19;}
    if (x) {return 20;}

    // short-circuiting chain of ||
    if (!(0 || 1 || (x = 1))) {return 21;}
    if (x) {return 22;}
    if (!(1 || (x = 1) || (x = 1))) {return 23;}
    if (x) {return 24;}

    return 0;
}
