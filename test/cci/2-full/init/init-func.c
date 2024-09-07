// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {
    int (*a)(void) = main;
    int (*b)(void) = &main;
    if (a != main) return 1;
    if (b != main) return 2;

    struct {
        int (*f)(void);
    } c = {main}, d = {&main};
    if (c.f != main) return 3;
    if (d.f != main) return 4;

    struct {
        struct {
            int (*f)(void);
        } s[4];
    } e = {
        main,
        {main},
        &main,
        {&main},
    };
    if (e.s[0].f != main) return 5;
    if (e.s[1].f != main) return 6;
    if (e.s[2].f != main) return 7;
    if (e.s[3].f != main) return 8;
}
