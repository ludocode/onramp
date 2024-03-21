// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TinyCC accepts this.
// MSVC compiles it to nonsense (it outputs the `)` as an extra token.)
// Kefir says "unexpected end of file" but otherwise correctly diagnoses it.
// GCC, Clang and chibicc correctly diagnose it as an error.
#define FOO() "include-hello.h"
#include FOO(
             )
