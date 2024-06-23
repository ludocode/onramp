// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// This declares `struct a` in global scope
struct a;

static struct a* get_a(void);
static int decode_a(struct a*);

int main(void) {

    // These are *not* forward declarations because of the type qualifier.
    // They're just useless declarations. GCC has a warning about useless
    // qualifiers in empty declarations, but it also has a stronger warning
    // about an incorrect forward declaration that fails to shadow an outer
    // declaration. NVC also passes this test (as does Onramp.)
    //
    // Clang currently seems to get this scoping rule wrong, but it also only
    // warns by default when passing incompatible struct pointers, so it still
    // runs this test correctly by coincidence. Interestingly, Clang passes
    // this test if `const` is at the end instead of the start, e.g. `struct a
    // const;`.
    //
    // cproc and LCC have the same behaviour as Clang except the pointer
    // conversion is an error instead of a warning so this test fails. Most
    // other toy compilers either don't handle tag scoping correctly, don't
    // bother to warn about mismatched pointers, or ignore type qualifiers.
    const struct a;

    // Call foo. This should *NOT* cause an unmatched type error because this
    // refers to the outer `struct a`, and there is no inner `struct a` yet.
    struct a* pa = get_a();
    int ret = decode_a(pa);

    // define inner struct; shadows outer struct
    struct a {
        int x;
    } a;
    (void)a;

    return ret;
}

// define outer struct. should be no conflict with inner struct.
struct a {
    char y;
} a;

static struct a* get_a(void) {
    return &a;
}

static int decode_a(struct a* a) {
    return a->y;
}
