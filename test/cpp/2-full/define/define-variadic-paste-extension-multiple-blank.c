// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Here's a fun one.
// GCC and Clang emit `1,`
// TinyCC and chibicc emit `1`
// MSVC emits `1` under the traditional preprocessor and `1,` under the new one
// LCC fails, even though it implements the `, ##` extension
// Kefir doesn't appear to implement the extension
#define FOO(x,...) x,##__VA_ARGS__
FOO(1,)
