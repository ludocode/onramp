// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// a bug that existed at one point

#ifdef FOO
/* comment */
#ifdef BAR
#endif
#endif
