// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define CONCAT(x, y) x ## y

// pasting happens before expansion of the arguments, so this doesn't work.
#define CONCAT3_BAD(a, b, c) CONCAT(a, CONCAT(b, c))
CONCAT3_BAD(bad, con, cat)

// this defers pasting, allowing the triple paste to work.
#define CONCAT3_GOOD(a, b, c) CONCAT3_GOOD_HELPER(a, CONCAT(b, c))
#define CONCAT3_GOOD_HELPER(a, bc) CONCAT(a, bc)
CONCAT3_GOOD(go, o, d)
