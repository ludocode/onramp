// The MIT License (MIT)
// Copyright (c) 2025-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    while (1) {
        defer {
            continue; // error, cannot continue out of defer
        }
    }
    return 1;
}
