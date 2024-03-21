// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// should be (1 ? 0 : (0 + 1))
#if 1 ? 0 : 0 + 1
nope
#endif

#if (1 ? 0 : 0) + 1
yup
#endif

// should be (0 ? (1 + 1) : 0)
#if 0 ? 1 + 1 : 0
nope
#endif
