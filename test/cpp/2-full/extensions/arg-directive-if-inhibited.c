// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define FOO 1
#define STRINGIFY(x) # x
STRINGIFY(
        #if 0
        0
        #else
        FOO
        #endif
        )
