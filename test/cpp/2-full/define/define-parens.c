// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define LP (
#define RP )
LP
RP
#define FOO(x) x
#define EXPAND(x) x
FOO LP 1 RP
EXPAND(FOO LP 1 RP)
