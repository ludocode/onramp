// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int x;     // ok, tentative definition
int x = 1; // ok, definition
int x;     // ok, tentative definition
int x = 2; // error, x already defined

int main(void) {}
