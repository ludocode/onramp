// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

/*
 * The Dave Prosser algorithm suggests, when expanding a function-like macro,
 * to intersect the hideset of the name of the macro with the closing
 * parenthesis of its arguments. The result is used as the hideset of all
 * expanded tokens.
 *
 * This is an example of where it matters. The token `XY` produced by A has
 * already used CONCAT, but the argument list `(Z, W)` has not. The
 * intersection is therefore empty and `CONCAT` can be used again.
 *
 * PCC fails this test. It works in all other compilers I've tried.
 */
#define CONCAT(a, b) a ## b
#define A(a, b, c, d) CONCAT(a, b)(c, d)
#define XY(a, b) CONCAT(a, b)
A(X, Y, Z, W)
