// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#ifndef __onramp__ // TODO enable on onramp later

#ifdef __onramp__
#include "bigint.h"
#endif

#ifndef __onramp__
    #include <stdint.h>
    #include <stddef.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    _Noreturn void __fatal(const char* s) {
        fprintf(stderr, "%s\n", s);
        abort();
    }

    // this test case can be compiled standalone with a native C compiler:
    // gcc -Wall -Wextra -fsanitize=address -g test/test_bigint.c -o /tmp/a && /tmp/a
    #include "../../../../core/libc/2-opc/src/bigint.c"
#endif

/**
 * Verifies that the given bigint matches the given string representation.
 */
void check(uint32_t n, uint32_t* x, const char* str, uint32_t line) {
    char buf[256];
    if (3 + n * 8 > sizeof(buf)) {
        __fatal("need bigger buffer");
    }
    __bigint_print_hex(n, x, buf, sizeof(buf));
    if (0 != strcmp(str, buf)) {
        fprintf(stderr, "failure at line %i:\n", line);
        fprintf(stderr, "    expected: %s\n", str);
        fprintf(stderr, "    actual:   %s\n", buf);
        abort();
    }
}

#define CHECK(n, x, str) check(n, x, str, __LINE__)

static void test_bigint_print(void) {
    uint32_t x[3];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    CHECK(3, x, "0x000000000000000000000000");

    x[0] = 0;
    x[1] = 1;
    x[2] = 0;
    CHECK(3, x, "0x000000000000000100000000");

    x[0] = 0x12345678;
    x[1] = 0x9abcdef0;
    x[2] = 0x87654321;
    CHECK(3, x, "0x876543219abcdef012345678");

    x[0] = 0xffffffff;
    x[1] = 0xffffffff;
    x[2] = 0xffffffff;
    CHECK(3, x, "0xffffffffffffffffffffffff");

    uint32_t y[7];
    y[0] = 0x12345678;
    y[1] = 0x23456789;
    y[2] = 0x3456789a;
    y[3] = 0x456789ab;
    y[4] = 0x56789abc;
    y[5] = 0x6789abcd;
    y[6] = 0x789abcde;
    CHECK(7, y, "0x789abcde6789abcd56789abc456789ab3456789a2345678912345678");
}

static void test_bigint_set_u32(void) {
    uint32_t x[3];

    __bigint_set_u32(3, x, 0);
    CHECK(3, x, "0x000000000000000000000000");

    __bigint_set_u32(3, x, 1);
    CHECK(3, x, "0x000000000000000000000001");

    __bigint_set_u32(3, x, 0x1234abcd);
    CHECK(3, x, "0x00000000000000001234abcd");

    __bigint_set_u32(3, x, 0xffffffff);
    CHECK(3, x, "0x0000000000000000ffffffff");
}

static void test_bigint_add(void) {
    uint32_t x[4];
    uint32_t y[4];
    uint32_t z[4];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 1;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 1;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000002");

    x[0] = 0x80000000;
    x[1] = 0x80000000;
    x[2] = 0x80000000;
    x[3] = 0x80000000;
    y[0] = 0x80000000;
    y[1] = 0x80000000;
    y[2] = 0x80000000;
    y[3] = 0x80000000;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000001000000010000000100000000");

    x[0] = 0xffffffff;
    x[1] = 0xffffffff;
    x[2] = 0xffffffff;
    x[3] = 0xffffffff;
    y[0] = 0xffffffff;
    y[1] = 0xffffffff;
    y[2] = 0xffffffff;
    y[3] = 0xffffffff;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0xfffffffffffffffffffffffffffffffe");

    // patterns
    x[0] = 0x12345678;
    x[1] = 0x4567abcd;
    x[2] = 0x89abcdef;
    x[3] = 0xabcdefab;
    y[0] = 0x12345678;
    y[1] = 0x4567abcd;
    y[2] = 0x89abcdef;
    y[3] = 0xabcdefab;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x579bdf5713579bde8acf579a2468acf0");

    // random numbers
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    y[0] = 0x5cfa0d5a;
    y[1] = 0x89935756;
    y[2] = 0x58e39b80;
    y[3] = 0xf5de01bf;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0xa126ecfd81e8f2ca51986841fbf65289");
}

static void test_bigint_sub(void) {
    uint32_t x[4];
    uint32_t y[4];
    uint32_t z[4];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 1;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0xffffffffffffffffffffffffffffffff");

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0xffffffff;
    y[1] = 0xffffffff;
    y[2] = 0xffffffff;
    y[3] = 0xffffffff;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000001");

    // matching patterns
    x[0] = 0x12345678;
    x[1] = 0x4567abcd;
    x[2] = 0x89abcdef;
    x[3] = 0xabcdefab;
    y[0] = 0x12345678;
    y[1] = 0x4567abcd;
    y[2] = 0x89abcdef;
    y[3] = 0xabcdefab;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    y[0] = 0x5cfa0d5a;
    y[1] = 0x89935756;
    y[2] = 0x58e39b80;
    y[3] = 0xf5de01bf;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0xb56ae97ed021bbc93e71b995420237d5");
}

static void test_bigint_shl(void) {
    uint32_t x[8];
    uint32_t z[8];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    __bigint_shl(4, z, x, 0);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 1);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 31);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 32);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 33);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 1000);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0x12345678;
    x[1] = 0x90abcdef;
    __bigint_shl(2, z, x, 0);
    CHECK(2, z, "0x90abcdef12345678");
    __bigint_shl(2, z, x, 1);
    CHECK(2, z, "0x21579bde2468acf0");
    __bigint_shl(2, z, x, 4);
    CHECK(2, z, "0x0abcdef123456780");
    __bigint_shl(2, z, x, 8);
    CHECK(2, z, "0xabcdef1234567800");
    __bigint_shl(2, z, x, 16);
    CHECK(2, z, "0xcdef123456780000");
    __bigint_shl(2, z, x, 31);
    CHECK(2, z, "0x891a2b3c00000000");
    __bigint_shl(2, z, x, 32);
    CHECK(2, z, "0x1234567800000000");
    __bigint_shl(2, z, x, 36);
    CHECK(2, z, "0x2345678000000000");
    __bigint_shl(2, z, x, 60);
    CHECK(2, z, "0x8000000000000000");
    __bigint_shl(2, z, x, 64);
    CHECK(2, z, "0x0000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    x[4] = 0x5cfa0d5a;
    x[5] = 0x89935756;
    x[6] = 0x58e39b80;
    x[7] = 0xf5de01bf;
    CHECK(8, x, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shl(8, z, x, 0);
    CHECK(8, z, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shl(8, z, x, 1);
    CHECK(8, z, "0xebbc037eb1c737011326aeacb9f41ab55691d67c520aae93900a21d73df88a5e");
    __bigint_shl(8, z, x, 4);
    CHECK(8, z, "0x5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f0");
    __bigint_shl(8, z, x, 8);
    CHECK(8, z, "0xde01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f00");
    __bigint_shl(8, z, x, 32);
    CHECK(8, z, "0x58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f00000000");
    __bigint_shl(8, z, x, 36);
    CHECK(8, z, "0x8e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f000000000");
    __bigint_shl(8, z, x, 60);
    CHECK(8, z, "0x0899357565cfa0d5aab48eb3e29055749c80510eb9efc452f000000000000000");
    __bigint_shl(8, z, x, 64);
    CHECK(8, z, "0x899357565cfa0d5aab48eb3e29055749c80510eb9efc452f0000000000000000");
    __bigint_shl(8, z, x, 124);
    CHECK(8, z, "0xaab48eb3e29055749c80510eb9efc452f0000000000000000000000000000000");
    __bigint_shl(8, z, x, 128);
    CHECK(8, z, "0xab48eb3e29055749c80510eb9efc452f00000000000000000000000000000000");
    __bigint_shl(8, z, x, 132);
    CHECK(8, z, "0xb48eb3e29055749c80510eb9efc452f000000000000000000000000000000000");
    __bigint_shl(8, z, x, 136);
    CHECK(8, z, "0x48eb3e29055749c80510eb9efc452f0000000000000000000000000000000000");
    __bigint_shl(8, z, x, 140);
    CHECK(8, z, "0x8eb3e29055749c80510eb9efc452f00000000000000000000000000000000000");
    __bigint_shl(8, z, x, 141);
    CHECK(8, z, "0x1d67c520aae93900a21d73df88a5e00000000000000000000000000000000000");
    __bigint_shl(8, z, x, 192);
    CHECK(8, z, "0xc80510eb9efc452f000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 196);
    CHECK(8, z, "0x80510eb9efc452f0000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 224);
    CHECK(8, z, "0x9efc452f00000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 252);
    CHECK(8, z, "0xf000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 256);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 0xffffffffu);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
}

static void test_bigint_shru(void) {
    uint32_t x[8];
    uint32_t z[8];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    __bigint_shru(4, z, x, 0);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 1);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 31);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 32);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 33);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 1000);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0x12345678;
    x[1] = 0x90abcdef;
    __bigint_shru(2, z, x, 0);
    CHECK(2, z, "0x90abcdef12345678");
    __bigint_shru(2, z, x, 1);
    CHECK(2, z, "0x4855e6f7891a2b3c");
    __bigint_shru(2, z, x, 4);
    CHECK(2, z, "0x090abcdef1234567");
    __bigint_shru(2, z, x, 32);
    CHECK(2, z, "0x0000000090abcdef");
    __bigint_shru(2, z, x, 36);
    CHECK(2, z, "0x00000000090abcde");
    __bigint_shru(2, z, x, 60);
    CHECK(2, z, "0x0000000000000009");
    __bigint_shru(2, z, x, 63);
    CHECK(2, z, "0x0000000000000001");
    __bigint_shru(2, z, x, 64);
    CHECK(2, z, "0x0000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    x[4] = 0x5cfa0d5a;
    x[5] = 0x89935756;
    x[6] = 0x58e39b80;
    x[7] = 0xf5de01bf;
    CHECK(8, x, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shru(8, z, x, 0);
    CHECK(8, z, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shru(8, z, x, 1);
    CHECK(8, z, "0x7aef00dfac71cdc044c9abab2e7d06ad55a4759f1482aba4e4028875cf7e2297");
    __bigint_shru(8, z, x, 4);
    CHECK(8, z, "0x0f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452");
    __bigint_shru(8, z, x, 8);
    CHECK(8, z, "0x00f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc45");
    __bigint_shru(8, z, x, 28);
    CHECK(8, z, "0x0000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9");
    __bigint_shru(8, z, x, 32);
    CHECK(8, z, "0x00000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb");
    __bigint_shru(8, z, x, 32);
    CHECK(8, z, "0x00000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb");
    __bigint_shru(8, z, x, 36);
    CHECK(8, z, "0x000000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510e");
    __bigint_shru(8, z, x, 128);
    CHECK(8, z, "0x00000000000000000000000000000000f5de01bf58e39b80899357565cfa0d5a");
    __bigint_shru(8, z, x, 132);
    CHECK(8, z, "0x000000000000000000000000000000000f5de01bf58e39b80899357565cfa0d5");
    __bigint_shru(8, z, x, 164);
    CHECK(8, z, "0x00000000000000000000000000000000000000000f5de01bf58e39b808993575");
    __bigint_shru(8, z, x, 252);
    CHECK(8, z, "0x000000000000000000000000000000000000000000000000000000000000000f");
    __bigint_shru(8, z, x, 256);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shru(8, z, x, 0xffffffffu);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
}

static void test_bigint_mul_u16(void) {
    uint32_t x[8];
    uint32_t z[8];

    x[0] = 0x10001;
    x[1] = 0x10001;
    CHECK(2, x, "0x0001000100010001");
    __bigint_mul_u16(2, z, x, 0);
    CHECK(2, z, "0x0000000000000000");
    __bigint_mul_u16(2, z, x, 1);
    CHECK(2, z, "0x0001000100010001");
    __bigint_mul_u16(2, z, x, 2);
    CHECK(2, z, "0x0002000200020002");
    __bigint_mul_u16(2, z, x, 4);
    CHECK(2, z, "0x0004000400040004");
    __bigint_mul_u16(2, z, x, 0xffff);
    CHECK(2, z, "0xffffffffffffffff");

    x[0] = 0xaaaaaaaa;
    x[1] = 0x55555555;
    CHECK(2, x, "0x55555555aaaaaaaa");
    __bigint_mul_u16(2, z, x, 0);
    CHECK(2, z, "0x0000000000000000");
    __bigint_mul_u16(2, z, x, 1);
    CHECK(2, z, "0x55555555aaaaaaaa");
    __bigint_mul_u16(2, z, x, 2);
    CHECK(2, z, "0xaaaaaaab55555554");
    __bigint_mul_u16(2, z, x, 0xffff);
    CHECK(2, z, "0x00005554ffff5556");

    // random numbers
    x[0] = 0xee2f25ae;
    x[1] = 0x656efc09;
    x[2] = 0x9c5718e6;
    x[3] = 0x59889bae;
    x[4] = 0x6716dc3a;
    CHECK(5, x, "0x6716dc3a59889bae9c5718e6656efc09ee2f25ae");
    __bigint_mul_u16(5, z, x, 0);
    CHECK(5, z, "0x0000000000000000000000000000000000000000");
    __bigint_mul_u16(5, z, x, 1);
    CHECK(5, z, "0x6716dc3a59889bae9c5718e6656efc09ee2f25ae");
    __bigint_mul_u16(5, z, x, 2);
    CHECK(5, z, "0xce2db874b311375d38ae31cccaddf813dc5e4b5c");
    __bigint_mul_u16(5, z, x, 3);
    CHECK(5, z, "0x354494af0c99d30bd5054ab3304cf41dca8d710a");
    __bigint_mul_u16(5, z, x, 10);
    CHECK(5, z, "0x06e49a477f5614d21b66f8fff655d8634dd778cc");
    __bigint_mul_u16(5, z, x, 0xfffe);
    CHECK(5, z, "0x0e0ca113e89d64f9e03833a2312bf61b494fb4a4");
    __bigint_mul_u16(5, z, x, 0xffff);
    CHECK(5, z, "0x75237d4e422600a87c8f4c88969af225377eda52");
}

#ifdef __GNUC__
static void test_bigint_mul_u16_loop() {
    for (size_t i = 0;; ++i) {
        if (i != 0 && (i % 1000000) == 0) printf("testing __bigint_mul_u16(): %zi...\n",i);

        typedef union {
            unsigned __int128 u128;
            uint32_t u32[4];
        } uu;

        // random 128-bit number
        uu l;
        l.u32[0] = arc4random();
        l.u32[1] = arc4random();
        l.u32[2] = arc4random();
        l.u32[3] = arc4random();

        // random 16-bit number
        uint16_t r = (uint16_t)arc4random();

        // expected result
        uu e = {.u128 = l.u128 * r};

        // actual result
        uu a;
        __bigint_mul_u16(4, a.u32, l.u32, r);

        if (e.u128 != a.u128) {
            printf("__bigint_mul_u16() FAILED:\n");
            char buf[256];
            __bigint_print_hex(4, l.u32, buf, sizeof(buf));
            printf("    left:     %s\n", buf);
            printf("    right:    %u\n", r);
            __bigint_print_hex(4, e.u32, buf, sizeof(buf));
            printf("    expected: %s\n", buf);
            __bigint_print_hex(4, a.u32, buf, sizeof(buf));
            printf("    actual:   %s\n", buf);
            exit(1);
        }
    }
}
#endif

#endif

int main(void) {
#ifndef __onramp__ // TODO enable on onramp later
    test_bigint_print();
    test_bigint_set_u32();
    test_bigint_add();
    test_bigint_sub();
    test_bigint_shl();
    test_bigint_shru();
    test_bigint_mul_u16();

    #ifdef __GNUC__
    (void)test_bigint_mul_u16_loop;
    //test_bigint_mul_u16_loop();
    #endif
#endif
}

