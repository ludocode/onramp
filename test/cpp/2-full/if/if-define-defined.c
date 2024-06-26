// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// Some fun compiler bugs here.
// GCC, Clang, MSVC and TinyCC correctly ignore and diagnose the attempt to define defined.
// chibicc and Kefir both allow it and both output "defined" and "1" but not "bar".

#define defined(FOO) 1

#if defined(BAR)
bar
#endif

#if defined(defined)
defined
#endif

defined(BAZ)
