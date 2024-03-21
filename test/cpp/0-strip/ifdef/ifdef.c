// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// we're stripping preprocessor directives so it is as though
// both conditions are true.
#ifdef foo
a
#endif
#ifndef foo
b
#endif
