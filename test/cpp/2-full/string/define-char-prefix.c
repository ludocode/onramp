// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// L is a valid character prefix so it is considered part of the character
// token. It is therefore not expanded.
#define L ERROR
static int l = L'l';

// X is not a valid character prefix so it is not considered part of
// the character token. It is therefore expanded.
#define X =
static int x X'x';

int main(void) {
    if (l != L'l') return 1;
    if (x != 'x') return 2;
}

// TODO this test case is designed to be runnable but Onramp doesn't support
// string/char prefixes yet so we compare to a .i file for now. The .i file
// should be deleted.
