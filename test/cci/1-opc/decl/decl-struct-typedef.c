// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

typedef struct foo {
    int a;
    int b;
    int c;
} foo_t;

int main(void) {
    if (sizeof(foo_t) != 12) {
        return 1;
    }
    return 0;
}
