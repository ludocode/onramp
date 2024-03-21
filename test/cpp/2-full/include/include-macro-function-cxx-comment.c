// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// results for this are the same as newline-before
#define FOO() "include-hello.h"
#include FOO//
            ()
