// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

// Make sure a declaration of an array of indeterminate length becomes a
// well-defined length after the definition.
extern char alice[];
char alice[6] = "Alice";

// Make sure a declaration of an array of definite length can be defined by an
// array of indeterminate length with excess initializers ignored
extern char bob[4];
char bob[] = "Bob\0!!!!!!"; // raises -Wexcess-initializers

// Make sure a declaration of an array of definite length can be defined by a
// short array of indeterminate length with the excess zeroed
extern char carl[9];
char carl[] = "Carl";

int main(void) {
    if (sizeof(alice) != 6) return 1;
    if (sizeof(bob) != 4) return 2;
    if (sizeof(carl) != 9) return 3;
    if (0 != strcmp(alice, "Alice")) return 4;
    if (0 != strcmp(bob, "Bob")) return 5;
    if (0 != strcmp(carl, "Carl")) return 6;
    if (0 != carl[6]) return 7;
    if (0 != carl[7]) return 8;
    if (0 != carl[8]) return 9;
}
