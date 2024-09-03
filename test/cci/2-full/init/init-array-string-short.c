// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    {
        // put some junk on the stack
        volatile int x = 0x12345678;
        volatile int y = 0x12345678;
    }
    char x[8] = "foo";
    if (x[0] != 'f') return 1;
    if (x[1] != 'o') return 2;
    if (x[2] != 'o') return 3;
    if (x[3] != 0) return 4;
    // make sure the rest of the array was zeroed
    if (x[4] != 0) return 5;
    if (x[5] != 0) return 6;
    if (x[6] != 0) return 7;
    if (x[7] != 0) return 8;
}
