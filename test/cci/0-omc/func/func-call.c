// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(int x) {
    return x;
}

int bar(void) {
    return foo(0);
}

int main(void) {
    return bar();
}
