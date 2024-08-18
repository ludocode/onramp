// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <stdbool.h>

bool foo(void) {
    return 0 == strcmp("foo", __func__);
}

bool baz(void) {
    return 0 == strcmp(__FUNCTION__, __func__);
}

int main(void) {
    if (!foo()) return 1;
    if (!baz()) return 3;

    // make sure there is only one instantiation of the name string
    // (this is required by the standard)
    if (__func__ != __func__) return false;

    // also make sure it's a char array (not a pointer to char)
    if (__func__ != &__func__) return false;
    if (sizeof(__func__) != 5) return false;
}
