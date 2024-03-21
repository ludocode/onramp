// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// pragma is not defined in any compilers.
#ifdef pragma
    a
#endif

// _Pragma is defined by Clang and GCC.
// It is not defined by MSVC, TinyCC, chibicc, Kefir or SCC.
#ifdef _Pragma
    b
#endif
