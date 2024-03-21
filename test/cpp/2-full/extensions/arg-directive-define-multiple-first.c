// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define A(x, y) x B
A(
    #define B 1
    ,
    #define B 2
)
