// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>

int main(void) {
    if (0 != getenv("__ONRAMP_ENV_VAR_DOES_NOT_EXIST"))
        return 1;

    // We're assuming PATH always exists on UNIX systems. Our VM spec does not
    // require any environment variables so we can't really check if one of
    // them exists. We could walk through the environment, pull out a key and
    // test it but that seems like overkill for this test.
    #ifdef __unix__
    if (0 == getenv("PATH"))
        return 2;
    #endif
}
