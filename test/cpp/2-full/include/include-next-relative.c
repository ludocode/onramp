// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// GCC, Kefir and TinyCC pass this test.
// Clang doesn't support #include_next on files found relative to the source
// file or by absolute path. It includes repeatedly until it hits the include
// limit emitting a warning each time.
// chibicc seems to go into an infinite loop.
src
#include "include-next.h"
