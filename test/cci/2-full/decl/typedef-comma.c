// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

typedef int **a, **b, **c;

int main(void) {
    if (!__builtin_types_compatible_p(a, b))
        return 1;
    if (!__builtin_types_compatible_p(a, c))
        return 1;
    return 0;
}
