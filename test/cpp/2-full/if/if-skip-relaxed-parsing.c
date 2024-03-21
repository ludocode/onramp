// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// C99 spec, section 6.10.2.4
// bug in SCC
#if 0
    #if FOO( * { ; 0123
        ignored
    #endif
#endif
