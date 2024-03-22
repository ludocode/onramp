// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Test forward declaration of variadic function
int open(const char* path, int flags, ...);

int open(const char* path, int flags, ...) {
    return 0;
}

int main(void) {
    return 0;
}
