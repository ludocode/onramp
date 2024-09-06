// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = {{{{{4}},}},};
    if (x != 4) return 1;
}
