// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define RUN(test) extern void test(void); test()

int main(void) {
    RUN(test_asserts);
    RUN(test_ctype);
    RUN(test_errno);
    RUN(test_internal);
    RUN(test_math);
    RUN(test_stdio_file);
    RUN(test_stdio_format);
    RUN(test_stdlib_malloc);
    RUN(test_stdlib_system);
    RUN(test_stdlib_util);
    RUN(test_string);
}
