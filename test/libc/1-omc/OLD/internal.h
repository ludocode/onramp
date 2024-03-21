// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

_Noreturn void __libc_error(const char* message);

void __assert_fail(const char* __expression, const char* __file,
        int __line, const char* __function);

int printf(const char* restrict format, ...);

// TODO message
#define libc_assert(expression, ...) \
    ((expression) ? ((void)0) : \
        __assert_fail(#expression, __FILE__, __LINE__, __func__))

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define CONST_CAST(T, expr) ((T)(expr))

#endif
