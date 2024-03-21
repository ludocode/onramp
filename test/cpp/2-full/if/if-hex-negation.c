// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// requires a 64-bit preprocessor
// note that the preprocessor must do all math as its longest supported type
// regardless of integer suffixes
#if ~1 == 0xFFFFFFFFFFFFFFFE
a
#endif
