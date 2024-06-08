// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // break in the expressions of a for/do/while/switch breaks out of the
    // outer statement, not the one being declared
    for (;;) {
        for (({break;});;)
            return 1;
        return 1;
    }
}
