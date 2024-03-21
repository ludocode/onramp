// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

/* Interestingly, this works in Clang and chibicc but not GCC or TinyCC.
 * There is apparently no way to include a file whose name has a " without a
 * preceding \ in any compiler. */
#include "include-\".h"
