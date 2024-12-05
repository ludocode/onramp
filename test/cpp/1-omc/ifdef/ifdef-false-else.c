// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    #ifdef UNDEFINED
        return 1;
    #else
        // This is the reason we explicitly check for `#else` when skipping
        // untaken conditional branches in cpp/1. If we ignore `#else`, both
        // branches will be skipped, and the preprocessor will silently output
        // incorrect code. We have to check to prevent this from being
        // compiled.
        return 0;
    #endif
    return 1;
}
