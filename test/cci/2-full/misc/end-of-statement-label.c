// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    {
        if (1)
            foo: // error: expected a statement
    }
    return 1;
}
