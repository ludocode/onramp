// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// We pass -fcommon which will give x weak linkage.
// (This test doesn't actually check that it links properly with multiple
// translation units, but the assembly output should have a weak sigil in it
// and the test compares the assembly.)
int x;
extern x;
int x;

// TODO this test is currently not working properly, we haven't implemented weak yet

int main(void) {
    if (x != 0) return 1;
}
