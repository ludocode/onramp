// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define FOO(x) a ## ## x   // ERROR: ## cannot appear twice in a row
FOO(b)
