// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// No compiler that I've tested supports concatenation of string literals in
// _Pragma. They all mangle the below.
_Pragma("onramp" "test" " " "\"a" "b\"")
