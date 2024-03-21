// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TinyCC accepts this.
// chibicc outputs no error message but exits with an error.
// GCC, Clang, MSVC and Kefir correctly diagnose the error.
#define FOO() "include-hello.h"
#include FOO
            ()
