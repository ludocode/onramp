// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

/* https://github.com/protopopov1122/kefir/issues/1 */
#define foo []
#define stringify(x) #x
const char* f = stringify(foo);
