// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This fails under chibicc.
// It works under GCC, Clang, MSVC, TinyCC and Kefir.
#define FOO
#include "include-hello.h" FOO
