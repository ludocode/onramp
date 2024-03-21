// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// error, not allowed in omC
#define FOO BAR
#define BAR FOO
FOO
