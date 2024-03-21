// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// necessary to make ghost_has() work
// bug in SCC
#define A(x) B_##x
#define B_1 1
#if A(1)
    a
#endif
