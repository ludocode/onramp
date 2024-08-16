// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x[1] = {1,};
    if (1 - x[0]) return 1;
    int y[2] = {1,2,};
    if (3 - y[0] - y[1]) return 2;
}
