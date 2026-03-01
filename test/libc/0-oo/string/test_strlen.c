// The MIT License (MIT)
// Copyright (c) 2023-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>

int main(void) {
    if (strlen("\0""123456") != 0) {return 27;}
    if (strlen("a\0""123456") != 1) {return 1;}
    if (strlen("ab\0""123456") != 2) {return 2;}
    if (strlen("abc\0""123456") != 3) {return 3;}
    if (strlen("abcd\0""123456") != 4) {return 4;}
    if (strlen("abcde\0""123456") != 5) {return 5;}
    if (strlen("abcdef\0""123456") != 6) {return 6;}
    if (strlen("abcdefg\0""123456") != 7) {return 7;}
    if (strlen("abcdefgh\0""123456") != 8) {return 8;}
    if (strlen("abcdefghi\0""123456") != 9) {return 9;}
    if (strlen("abcdefghij\0""123456") != 10) {return 10;}
    if (strlen("abcdefghijk\0""123456") != 11) {return 11;}
    if (strlen("abcdefghijkl\0""123456") != 12) {return 12;}
    if (strlen("abcdefghijklm\0""123456") != 13) {return 13;}
    if (strlen("abcdefghijklmn\0""123456") != 14) {return 14;}
    if (strlen("abcdefghijklmno\0""123456") != 15) {return 15;}
    if (strlen("abcdefghijklmnop\0""123456") != 16) {return 16;}
    if (strlen("abcdefghijklmnopq\0""123456") != 17) {return 17;}
    if (strlen("abcdefghijklmnopqr\0""123456") != 18) {return 18;}
    if (strlen("abcdefghijklmnopqrs\0""123456") != 19) {return 19;}
    if (strlen("abcdefghijklmnopqrst\0""123456") != 20) {return 20;}
    if (strlen("abcdefghijklmnopqrstu\0""123456") != 21) {return 21;}
    if (strlen("abcdefghijklmnopqrstuv\0""123456") != 22) {return 22;}
    if (strlen("abcdefghijklmnopqrstuvw\0""123456") != 23) {return 23;}
    if (strlen("abcdefghijklmnopqrstuvwx\0""123456") != 24) {return 24;}
    if (strlen("abcdefghijklmnopqrstuvwxy\0""123456") != 25) {return 25;}
    if (strlen("abcdefghijklmnopqrstuvwxyz\0""123456") != 26) {return 26;}
    if (strlen("\0\0\0\0\0\0") != 0) {return 57;}
    if (strlen("a\0\0\0\0\0\0") != 1) {return 31;}
    if (strlen("ab\0\0\0\0\0\0") != 2) {return 32;}
    if (strlen("abc\0\0\0\0\0\0") != 3) {return 33;}
    if (strlen("abcd\0\0\0\0\0\0") != 4) {return 34;}
    if (strlen("abcde\0\0\0\0\0\0") != 5) {return 35;}
    if (strlen("abcdef\0\0\0\0\0\0") != 6) {return 36;}
    if (strlen("abcdefg\0\0\0\0\0\0") != 7) {return 37;}
    if (strlen("abcdefgh\0\0\0\0\0\0") != 8) {return 38;}
    if (strlen("abcdefghi\0\0\0\0\0\0") != 9) {return 39;}
    if (strlen("abcdefghij\0\0\0\0\0\0") != 10) {return 40;}
    if (strlen("abcdefghijk\0\0\0\0\0\0") != 11) {return 41;}
    if (strlen("abcdefghijkl\0\0\0\0\0\0") != 12) {return 42;}
    if (strlen("abcdefghijklm\0\0\0\0\0\0") != 13) {return 43;}
    if (strlen("abcdefghijklmn\0\0\0\0\0\0") != 14) {return 44;}
    if (strlen("abcdefghijklmno\0\0\0\0\0\0") != 15) {return 45;}
    if (strlen("abcdefghijklmnop\0\0\0\0\0\0") != 16) {return 46;}
    if (strlen("abcdefghijklmnopq\0\0\0\0\0\0") != 17) {return 47;}
    if (strlen("abcdefghijklmnopqr\0\0\0\0\0\0") != 18) {return 48;}
    if (strlen("abcdefghijklmnopqrs\0\0\0\0\0\0") != 19) {return 49;}
    if (strlen("abcdefghijklmnopqrst\0\0\0\0\0\0") != 20) {return 50;}
    if (strlen("abcdefghijklmnopqrstu\0\0\0\0\0\0") != 21) {return 51;}
    if (strlen("abcdefghijklmnopqrstuv\0\0\0\0\0\0") != 22) {return 52;}
    if (strlen("abcdefghijklmnopqrstuvw\0\0\0\0\0\0") != 23) {return 53;}
    if (strlen("abcdefghijklmnopqrstuvwx\0\0\0\0\0\0") != 24) {return 54;}
    if (strlen("abcdefghijklmnopqrstuvwxy\0\0\0\0\0\0") != 25) {return 55;}
    if (strlen("abcdefghijklmnopqrstuvwxyz\0\0\0\0\0\0") != 26) {return 56;}

    if (strlen(0 + "\0""123456") != 0) {return 61;}
    if (strlen(1 + "a\0""123456") != 0) {return 62;}
    if (strlen(2 + "ab\0""123456") != 0) {return 63;}
    if (strlen(3 + "abc\0""123456") != 0) {return 64;}

    if (strlen(0 + "a\0\0\0\0\0\0") != 1) {return 65;}
    if (strlen(1 + "ab\0\0\0\0\0\0") != 1) {return 66;}
    if (strlen(2 + "abc\0\0\0\0\0\0") != 1) {return 67;}
    if (strlen(3 + "abcd\0\0\0\0\0\0") != 1) {return 68;}

    if (strlen(0 + "ab\0\0\0\0\0\0") != 2) {return 69;}
    if (strlen(1 + "abc\0\0\0\0\0\0") != 2) {return 70;}
    if (strlen(2 + "abcd\0\0\0\0\0\0") != 2) {return 71;}
    if (strlen(3 + "abcde\0\0\0\0\0\0") != 2) {return 72;}

    if (strlen(0 + "abc\0\0\0\0\0\0") != 3) {return 73;}
    if (strlen(1 + "abcd\0\0\0\0\0\0") != 3) {return 74;}
    if (strlen(2 + "abcde\0\0\0\0\0\0") != 3) {return 75;}
    if (strlen(3 + "abcdef\0\0\0\0\0\0") != 3) {return 76;}

    if (strlen(0 + "abcdefghijk\0\0\0\0\0\0") != 11) {return 77;}
    if (strlen(1 + "abcdefghijk\0\0\0\0\0\0") != 10) {return 78;}
    if (strlen(2 + "abcdefghijk\0\0\0\0\0\0") != 9) {return 79;}
    if (strlen(3 + "abcdefghijk\0\0\0\0\0\0") != 8) {return 80;}
}
