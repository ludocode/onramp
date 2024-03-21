// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

enum foo {
    bar = 10,
};

int main(void) {
    if (bar != 10) {
        return 1;
    }
    return 0;
}
