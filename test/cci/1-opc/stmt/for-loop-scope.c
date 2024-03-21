// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    for (int x = 1; x < 4; x = (x + 1)) {}
    return x; // error, x should not be reachable
}
