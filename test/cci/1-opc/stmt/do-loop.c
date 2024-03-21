// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int count = 5;
    int total = 0;
    do {
        count = (count - 1);
        total = (total + 2);
    } while (count > 0);
    if (total != 10) {
        return 1;
    }
    return 0;
}
