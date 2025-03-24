// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    while (1) {
        defer {
            return 2; // error, cannot return out of defer
        }
    }
    return 1;
}
