// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <strings.h>

// TODO test ordering
// TODO test more stuff, empty strings etc

int main(void) {
    if (0 != strcasecmp("hello", "Hello")) return 1;
    if (0 == strcasecmp("hello", "goodbye")) return 2;
}
