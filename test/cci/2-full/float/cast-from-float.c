// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    float f = 1.0f;
    int i1 = f;
    if (i1 != 1) return 1;

    f = 2.0f;
    long long ll2 = f;
    if (ll2 != 2) return 2;
}
