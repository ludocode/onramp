// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    const char* foo = "├└"; // box drawing chars used in cci/2 node_print_tree()
    if (foo[0] != (char)0xe2) return 1;
    if (foo[1] != (char)0x94) return 2;
    if (foo[2] != (char)0x9c) return 3;
    if (foo[3] != (char)0xe2) return 4;
    if (foo[4] != (char)0x94) return 5;
    if (foo[5] != (char)0x94) return 6;
}
