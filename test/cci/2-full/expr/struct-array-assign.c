// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

struct s {
    short a,b,c,d,e;
};

int main(void) {
    if (sizeof(struct s) != 10) return 1;
    struct s array[4];

    struct s val = {1,2,3,4,5};

    array[0] = val;
    if (0 != memcmp(array, &val, sizeof(struct s))) return 2;

    array[1] = val;
    if (0 != memcmp(&array[1], &val, sizeof(struct s))) return 3;

    array[2] = val;
    if (0 != memcmp(array + 2, &val, sizeof(struct s))) return 4;

    array[3] = val;
    if (0 != memcmp((char*)array + sizeof(array) - sizeof(val), &val, sizeof(struct s))) return 5;
}
