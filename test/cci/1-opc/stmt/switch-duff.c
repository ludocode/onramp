// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    char buf[128];
    const char* str = "The quick brown fox jumps over the lazy dog.";

    const char* to = buf;
    const char* from = str;
    size_t count = (strlen(str) + 1);

    // duff's device
    int n = ((count + 7) / 8);
    switch (count & 7) {
        case 0: do { *to++ = *from++;
        case 7:      *to++ = *from++;
        case 6:      *to++ = *from++;
        case 5:      *to++ = *from++;
        case 4:      *to++ = *from++;
        case 3:      *to++ = *from++;
        case 2:      *to++ = *from++;
        case 1:      *to++ = *from++;
                } while ((n = (n - 1)) > 0);
    }

    if (0 != strcmp(buf, str)) {
        return 1;
    }

    return 0;
}
