// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int x = 0;
    if (0)
        return 1;
    else if (1)
        x = 1;
    else
        return 1;
    if (x != 1)
        return 1;

    x = 0;
    if (1)
        x = 1;
    else if (1)
        return 1;
    else
        return 1;
    if (x != 1)
        return 1;

    x = 0;
    if (0)
        return 1;
    else if (0)
        return 1;
    else
        x = 1;
    if (x != 1)
        return 1;

    return 0;
}
