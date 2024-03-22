// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

int main(void) {


    // test signed

    // decimal
    int dec0 = 12345678;
    int dec1 = 98765432L;

    // hexadecimal
    int hex0 = 0X01234567;
    int hex1 = 0xAbCdEf89l;

    // octal needs to be supported
    int oct0 = 0755;
    int oct1 = 00000644l;


    // test unsigned

    // decimal
    int udec0 = 12345678u;
    int udec1 = 98765432ul;
    int udec2 = 98765432uL;
    int udec3 = 98765432Ul;
    int udec4 = 98765432UL;

    // hexadecimal
    int uhex0 = 0x01234567lu;
    int uhex1 = 0XaBcDeF89LU;

    // octal needs to be supported
    int uoct0 = 0755lU;
    int uoct1 = 00000644Lu;


    // test that unsigned math works correctly
    if (4294967292u / 1073741823u != 4) {
        return 1;
    }

    return 0;
}
