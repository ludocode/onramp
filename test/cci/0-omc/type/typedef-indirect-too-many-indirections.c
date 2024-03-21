// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// A maximum of 15 indirections are supported.
typedef int******** foo;
typedef foo******** bar; // ERROR: too many indirections
