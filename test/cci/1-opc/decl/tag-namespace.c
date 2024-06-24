// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// The tag namespace is shared between structs, unions and enums.

struct foo {
    int x;
};
union foo { // ERROR: duplicate tag declared in this scope
    int y;
};

// (we don't test `enum` because cci/1 ignores the tag of an enum.)

int main(void) {
    return 0;
}
