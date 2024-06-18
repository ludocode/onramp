// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>

int main(void) {
    if (0)
        if (1)
            return 1;
        else
            return 1;
    else
        return 0;
}
