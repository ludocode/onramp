// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int foo(void) {
    return strcmp("foo", __func__);
}

int bar(void) {
    // make sure there is only one instantiation of the name string
    return __func__ != __func__;
}

int baz(void) {
    return strcmp(__FUNCTION__, __func__);
}

int main(void) {
    if (foo()) return 1;
    if (bar()) return 2;
    if (baz()) return 3;
}
