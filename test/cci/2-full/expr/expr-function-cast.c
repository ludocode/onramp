// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int foo(void) {
    return 5;
}

int main(void) {

    // explicit casts
    if (5 != ((int(*)(void))foo)()) return 1;
    if (5 != ((int(*)(void))(void*)foo)()) return 2;

    // implicit casts
    void* f = foo;
    int (*g)(void) = f;
    if (5 != g()) return 3;

    // undefined behaviour but we want to make sure this works because
    // real programs do this
    #ifdef __onramp__
    #ifdef __onramp_cci__
    ((void(*)(void))foo)();
    #endif
    #endif
}
