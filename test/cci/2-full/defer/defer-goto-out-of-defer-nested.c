// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    {
        {
            defer {
                goto foo; // error, cannot goto out of defer
            }
        }
    }
foo:
    return 1;
}
