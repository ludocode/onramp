// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    switch (2ll) {
        case 1ll: return 1;
        case 2ll: break;
        case 3ll: return 2;
        default: return 3;
    }

    switch (4ull) {
        case 4ull: break;
        case 5ull: return 4;
        default: return 5;
    }

    switch (6ll) {
        case 5ll: return 6;
        case 7ll: return 7;
        // no default, no match
    }

    switch (-8ll) {
        //case -9: return 8;
        case -8: break;
        default: return 9;
    }

    switch (1000000) {
        case 2000000ll: return 10;
        case 1000000ll: break;
        case 3000000ll: return 11;
    }
}
