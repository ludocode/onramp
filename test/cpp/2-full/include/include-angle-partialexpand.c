// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// All compilers fail this with trying to include <foo/a/foo-path.h>
#define include foo
#define FOO <include/a/include-path.h>
#include FOO
