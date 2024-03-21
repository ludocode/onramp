// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

/* https://github.com/protopopov1122/kefir/issues/1 */
#define foo []
#define cat(x, y) x##y
int cat(foo,_);
