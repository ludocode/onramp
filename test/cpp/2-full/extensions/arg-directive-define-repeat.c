// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define A(x) B x B x B
A(
    #ifdef B
        #undef B
        #define B 2
        def
    #else
        #define B 1
        nodef
    #endif
)
