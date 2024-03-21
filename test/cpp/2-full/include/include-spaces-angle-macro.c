// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This fails in all compilers I've tested:
// GCC, Clang, MSVC, TinyCC, chibicc, Kefir all fail.
#define FOO <include  spaces.h>
#include FOO
