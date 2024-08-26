// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    if (6 != sizeof("Hello")) return 1;
    if (13 != sizeof(
                "Hello"
                " "
                "world"
                "!")) return 2;
}
