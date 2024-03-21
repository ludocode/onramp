// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Clang and MSVC include the correct file.
// GCC, LCC, chibicc and Kefir try to include <foo/a/foo-path.h>
#define include foo
#define FOO /*nothing*/
#include FOO <include/a/include-path.h>
