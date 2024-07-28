// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // make sure x in statement expression doesn't conflict with x declared
    // outside of it
    int x = ({
        int x = 3;
        x;
    });
    return 5 - x - 2;
}
