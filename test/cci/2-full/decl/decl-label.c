// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
foo:
    int x; // ERROR: A label cannot precede a declaration (before C23.)
    return 1;
}
