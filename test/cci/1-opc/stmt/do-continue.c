// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int fail = 0;
    int pass = 0;
    do {
        if (pass) return 0;
        if (fail) return 1;
        ++fail;
        continue;
    } while (pass += 1);
}
