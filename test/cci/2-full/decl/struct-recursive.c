// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct foo {
    struct foo* foo; // this should not cause a memory leak
};

int main(void) {
}
