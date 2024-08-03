// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {

    // switch with no case statements is skipped
    switch (1) {
        return 1;
    }

    // but we can still jump into one
    switch (1) {
        foo:
            return 0;
    }
    goto foo;

    return 2;
}
