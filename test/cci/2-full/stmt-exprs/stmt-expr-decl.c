// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    // If a declaration is the last statement of a statement expression, the
    // statement expression has type void. The statement expression does *not*
    // evaluate to the value of the variable being declared.
    return ({int x = 0;}); // ERROR, cannot convert void to int
}
