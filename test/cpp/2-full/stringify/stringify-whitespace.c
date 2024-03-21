// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define A(x) # x
A(a)
#define B(x) #/*hello*/x
B(b)
#define C(x) # /*hello*/ /*world*/ x
C(c)
