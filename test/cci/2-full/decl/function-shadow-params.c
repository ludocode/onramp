// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int x) {
    int x = 3; // ERROR, cannot shadow a parameter at function scope
    return x;
}

int main(void) {
    return foo(2);
}
