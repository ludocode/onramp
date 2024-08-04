// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void* p) {
    return 0;
}

int main(void) {
    return foo((void**)0);
}
