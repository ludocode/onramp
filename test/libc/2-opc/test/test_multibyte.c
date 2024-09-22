// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>
#include <stdint.h>

static void test_mbtowc(void) {
    wchar_t wchar;

    // null source ignores other params
    if (0 != mbtowc(NULL, NULL, 0)) exit(1);
    if (0 != mbtowc((void*)4, NULL, SIZE_MAX)) exit(1);

    // ASCII
    char a[] = {'a'};
    if (1 != mbtowc(NULL, a, sizeof(a))) exit(1);
    if (1 != mbtowc(&wchar, a, sizeof(a))) exit(1);
    if (wchar != 'a') exit(1);

    // null byte converts normally
    char null[] = {0};
    if (1 != mbtowc(NULL, null, sizeof(null))) exit(1);
    if (1 != mbtowc(&wchar, null, sizeof(null))) exit(1);
    if (wchar != 0) exit(1);

    // greek small letter alpha
    char alpha[] = {0xCE, 0xB1};
    if (2 != mbtowc(NULL, alpha, sizeof(alpha))) exit(1);
    if (2 != mbtowc(&wchar, alpha, sizeof(alpha))) exit(1);
    if (wchar != 0x3B1u) exit(1);

    // hiragana letter small a
    char ha[] = {0xE3, 0x81, 0x81};
    if (3 != mbtowc(NULL, ha, sizeof(ha))) exit(1);
    if (3 != mbtowc(&wchar, ha, sizeof(ha))) exit(1);
    if (wchar != 0x3041u) exit(1);

    // grinning face
    char face[] = {0xF0, 0x9F, 0x98, 0x80};
    if (4 != mbtowc(NULL, face, sizeof(face))) exit(1);
    if (4 != mbtowc(&wchar, face, sizeof(face))) exit(1);
    if (wchar != 0x1F600u) exit(1);

    // truncated 4-byte sequence, grinning face
    char truncated[] = {0xF0, 0x9F, 0x98};
    if (-1 != mbtowc(NULL, truncated, sizeof(truncated))) exit(1);
    wchar = 7;
    if (-1 != mbtowc(&wchar, truncated, sizeof(truncated))) exit(1);
    if (wchar != 7) exit(1);

    // overlong 2-byte sequence, the null byte in Java Modified UTF-8
    char overlong[] = {0xC0, 0x80};
    if (-1 != mbtowc(NULL, overlong, sizeof(overlong))) exit(1);
    wchar = 13;
    if (-1 != mbtowc(&wchar, overlong, sizeof(overlong))) exit(1);
    if (wchar != 13) exit(1);
}

static void test_wctomb(void) {
    char buf[4];
    char canary = 137;

    // ASCII
    buf[0] = 13;
    buf[1] = 7;
    if (1 != wctomb(NULL, 'a')) exit(1);
    if (1 != wctomb(buf, 'a')) exit(1);
    if (buf[0] != 'a') exit(1);
    if (buf[1] != 7) exit(1);

    // null byte converts normally
    buf[0] = 17;
    buf[1] = 4;
    if (1 != wctomb(NULL, 0)) exit(1);
    if (1 != wctomb(buf, 0)) exit(1);
    if (buf[0] != 0) exit(1);
    if (buf[1] != 4) exit(1);

    // greek small letter alpha
    buf[2] = 23;
    if (2 != wctomb(NULL, 0x3B1u)) exit(1);
    if (2 != wctomb(buf, 0x3B1u)) exit(1);
    if (buf[0] != (char)0xCE) exit(1);
    if (buf[1] != (char)0xB1) exit(1);
    if (buf[2] != (char)23) exit(1);

    // hiragana letter small a
    buf[3] = 9;
    if (3 != wctomb(NULL, 0x3041u)) exit(1);
    if (3 != wctomb(buf, 0x3041u)) exit(1);
    if (buf[0] != (char)0xE3) exit(1);
    if (buf[1] != (char)0x81) exit(1);
    if (buf[2] != (char)0x81) exit(1);
    if (buf[3] != (char)9) exit(1);

    // grinning face
    if (4 != wctomb(NULL, 0x1F600u)) exit(1);
    if (4 != wctomb(buf, 0x1F600u)) exit(1);
    if (buf[0] != (char)0xF0) exit(1);
    if (buf[1] != (char)0x9F) exit(1);
    if (buf[2] != (char)0x98) exit(1);
    if (buf[3] != (char)0x80) exit(1);
    if (canary != (char)137) exit(1);

    // out of range
    buf[0] = 73;
    if (-1 != wctomb(NULL, 0x110000)) exit(1);
    if (-1 != wctomb(buf, 0x110000)) exit(1);
    if (buf[0] != (char)73) exit(1);
}

int main(void) {
    test_mbtowc();
    test_wctomb();
}
