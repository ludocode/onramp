// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO include libc headers (probably?)
//#include <string.h>
int strcmp(char*, char*);

int main(void) {
    if (0 != strcmp(__FILE__, "./misc/line-file.c")) return 1;
    #line 99
    if (0 != strcmp(__FILE__, "./misc/line-file.c")) return 2;
    #line 15 "some_file.with_long_extension"
    if (0 != strcmp(__FILE__, "some_file.with_long_extension")) return 3;
    #line 123 "filename with spaces"
    if (0 != strcmp(__FILE__, "filename with spaces")) return 5;
    #line 1 "/absolute/path"
    if (0 != strcmp(__FILE__, "/absolute/path")) return 5;
}
