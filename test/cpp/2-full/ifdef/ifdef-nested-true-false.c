// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define A
#ifdef A
    a
    #ifdef B
        b
    #else
        c
    #endif
#else
d
#endif
