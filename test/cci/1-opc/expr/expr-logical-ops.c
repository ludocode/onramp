// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;

    // basic test of &&
    if (0 && 0) {return 1;}
    if (0 && 1) {return 1;}
    if (1 && 0) {return 1;}
    if (!(1 && 1)) {return 1;}

    // basic test of ||
    if (0 || 0) {return 1;}
    if (!(0 || 1)) {return 1;}
    if (!(1 || 0)) {return 1;}
    if (!(1 || 1)) {return 1;}

    // short-circuiting &&
    if (0 && (x = 1)) {return 1;}
    if (x) {return 1;}
    if (!(1 && (x = 1))) {return 1;}
    if (!x) {return 1;}
    x = 0;

    // short-circuiting ||
    if (!(1 || (x = 1))) {return 1;}
    if (x) {return 1;}
    if (!(0 || (x = 1))) {return 1;}
    if (!x) {return 1;}
    x = 0;

    // short-circuiting chain of &&
    if (1 && 0 && (x = 1)) {return 1;}
    if (x) {return 1;}
    if (0 && (x = 1) && (x = 1)) {return 1;}
    if (x) {return 1;}

    // short-circuiting chain of ||
    if (!(0 || 1 || (x = 1))) {return 1;}
    if (x) {return 1;}
    if (!(1 || (x = 1) || (x = 1))) {return 1;}
    if (x) {return 1;}

    return 0;
}
