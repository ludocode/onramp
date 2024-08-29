// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int cmp(const char*, const char*) __asm__("strcmp");
    if (0 != cmp("hello", "hello"))
        return 1;
    if (0 >= cmp("helzo", "hello"))
        return 1;
    if (0 <= cmp("bello", "hello"))
        return 1;
    return 0;
}
