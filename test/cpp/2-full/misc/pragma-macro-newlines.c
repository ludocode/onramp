// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Tests that the preprocessor will correctly insert newlines when converting
// _Pragma to #pragma
#define FOO A _Pragma("mark") B
FOO
