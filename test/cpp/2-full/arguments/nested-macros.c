// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define FOO(x) BAR(BAZ(x))
#define BAR(y) 1 y -1
#define BAZ(z) 2 z -2
FOO(w)
