// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(short, short x, short, short, short y) {
    return 18 - x - y;
}

int main(int, char*) {
    return foo(1, 2, 4, 8, 16);
}
