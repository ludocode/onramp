// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int count = 0;
    do count = (count + 1); while (0);
    if (count != 1) {
        return 1;
    }
    return 0;
}
