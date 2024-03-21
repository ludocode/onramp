// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

/* https://github.com/rui314/chibicc/issues/99 */
#define X typedef int unused; Y
#define Y
X
/* comment */
#define Z
