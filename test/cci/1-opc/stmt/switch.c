// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // typical switch
    int x = 4;
    switch (x * 2) {
        case 10: return 1;
        default: return 2;
        case 19: return 3;
        case 2*2+2*2: break;
        return 4;
    }

    // switch with no default and no match
    switch ('x') {
        case 'a': return 5;
        case 'b': return 6;
        case 'c': return 7;
    }

    // switch fallthrough
    switch (x = 0, 'b') {
        case 'a': return 8;
        case 'b': x = (x + 1);
        case 'c': x = (x + 1);
        default: x = (x + 1);
    }
    if (x != 3) {
        return 9;
    }

    // switch without braces, match
    x = 1;
    switch (0)
        case 0:
            x = 0;
    if (x) {return 10;}

    // switch without braces, no match
    switch (0)
        case 1:
            return 11;

    // nested switch
    switch (0)
        switch (0)
            default: return 12;

    return 0;
}
