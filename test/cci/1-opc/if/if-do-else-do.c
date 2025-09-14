// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// test `do` (braced and unbraced) as statement of unbraced `if` and `else`

int main(void) {

    // braced `do`

    if (1)
        do {} while (0);
    else
        do {return 1;} while (0);

    if (0)
        do {return 2;} while (0);
    else
        do {} while (0);


    // unbraced `do`

    if (1)
        do ; while (0);
    else
        do return 3; while (0);

    if (0)
        do return 4; while (0);
    else
        do ; while (0);

}
