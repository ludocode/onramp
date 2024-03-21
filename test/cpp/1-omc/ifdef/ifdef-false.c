// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#ifdef foo
a
#endif
#ifdef bar


// test of line numbers in untaken conditional branches


b



// test of token pasting in untaken branches
foo ## bar
#define x foo ## bar



// test of invalid preprocessor directives in untaken conditional branches
#foo(#{}%${<
#


#endif
c
