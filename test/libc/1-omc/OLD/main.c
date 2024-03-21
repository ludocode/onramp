// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define RUN(test) extern void test(void); test()

void __malloc_init(void);

int main(void) {
    __malloc_init();
    RUN(test_stdlib_malloc);
}
