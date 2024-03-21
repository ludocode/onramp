// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo;

int main(void) {
    return sizeof(struct foo); // ERROR, forward-declared struct has no size
}
