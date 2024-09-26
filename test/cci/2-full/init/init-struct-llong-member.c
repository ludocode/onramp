// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct s {
    long long ll;
};

int main(void) {
    struct s s = {4}; // not 4LL; testing implicit cast from int to long long in initializer
    if (4 != s.ll) return 1;
}
