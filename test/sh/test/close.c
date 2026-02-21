// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdio.h>

int main(void) {
    // Close stdout. When run in the Onramp shell, this must not close
    // the shell's stdout.
    fclose(stdout);
}
