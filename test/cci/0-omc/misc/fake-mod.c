// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdio.h>
#include <stdlib.h>

int mod(int left, int right) {
	return (left - ((left/right) * right));
}

int main(void) {
    if (mod(7,3) != 1) {
        return 1;
    }
    if (mod(15, 4) != 3) {
        return 1;
    }
	if (mod(500, 10) != 0) {
        return 1;
    }
    return 0;
}
