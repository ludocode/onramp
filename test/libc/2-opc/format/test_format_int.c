// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[128];

    #define TEST_FORMAT(ret, expected, ...) \
        memset(buf, 0xEE, sizeof(buf)); \
        snprintf(buf, sizeof(buf), __VA_ARGS__); \
        if (0 != strcmp(expected, buf)) { \
            printf("bad printf %i. expected \"%s\", got \"%s\"\n", ret, expected, buf); \
            return ret; \
        }

    // basic number formatting
    TEST_FORMAT(1, "5", "%i", 5);
    TEST_FORMAT(2, "35", "%i", 35);
    TEST_FORMAT(3, "0", "%i", 0);
    TEST_FORMAT(4, "-7", "%i", -7);
    TEST_FORMAT(5, "-293", "%i", -293);
    TEST_FORMAT(6, "2147483647", "%i", INT_MAX);
    TEST_FORMAT(7, "-2147483648", "%i", INT_MIN);

    // sign flags ('+' and ' ')
    TEST_FORMAT(10, "+221", "%+i", 221);
    TEST_FORMAT(11, "-1971", "%+i", -1971);
    TEST_FORMAT(12, "+0", "%+i", 0);
    TEST_FORMAT(13, " 221", "% i", 221);
    TEST_FORMAT(14, "-1971", "% i", -1971);
    TEST_FORMAT(15, " 0", "% i", 0);

    // sign flags combined. '+' overrides ' '
    TEST_FORMAT(20, "+221", "%+ i", 221);
    TEST_FORMAT(21, "-1971", "%+ i", -1971);
    TEST_FORMAT(22, "+0", "%+ i", 0);
    TEST_FORMAT(23, "+221", "% +i", 221);
    TEST_FORMAT(24, "-1971", "% +i", -1971);
    TEST_FORMAT(25, "+0", "% +i", 0);

    // basic field width
    TEST_FORMAT(30, "   27", "%5i", 27);
    TEST_FORMAT(31, "2147483647", "%5i", INT_MAX);
    TEST_FORMAT(32, "     2147483647", "%15i", INT_MAX);
    TEST_FORMAT(33, "    -2147483648", "%15i", INT_MIN);
    TEST_FORMAT(34, "                                              2999", "%50i", 2999);
    TEST_FORMAT(35, "-123", "%1i", -123);

    // field width left-aligned ('-' flag)
    TEST_FORMAT(40, "27   ", "%-5i", 27);
    TEST_FORMAT(41, "2147483647", "%-5i", INT_MAX);
    TEST_FORMAT(42, "2147483647     ", "%-15i", INT_MAX);
    TEST_FORMAT(43, "-2147483648    ", "%-15i", INT_MIN);
    TEST_FORMAT(44, "2999                                              ", "%-50i", 2999);
    TEST_FORMAT(45, "-123", "%-1i", -123);

    // field width left-aligned with plus sign ('-' and '+')
    TEST_FORMAT(50, "+27  ", "%-+5i", 27);
    TEST_FORMAT(51, "+2147483647", "%-+5i", INT_MAX);
    TEST_FORMAT(52, "+2147483647    ", "%-+15i", INT_MAX);
    TEST_FORMAT(53, "-2147483648    ", "%-+15i", INT_MIN);
    TEST_FORMAT(54, "+2999                                             ", "%-+50i", 2999);
    TEST_FORMAT(55, "-123", "%-+1i", -123);

    // field width left-aligned with space for sign ('-' and ' ')
    TEST_FORMAT(60, " 27  ", "%- 5i", 27);
    TEST_FORMAT(61, " 2147483647", "%- 5i", INT_MAX);
    TEST_FORMAT(62, " 2147483647    ", "%- 15i", INT_MAX);
    TEST_FORMAT(63, "-2147483648    ", "%- 15i", INT_MIN);
    TEST_FORMAT(64, " 2999                                             ", "%- 50i", 2999);
    TEST_FORMAT(65, "-123", "%- 1i", -123);

    // field width and plus sign
    TEST_FORMAT(70, "  +27", "%+5i", 27);
    TEST_FORMAT(71, "+2147483647", "%+5i", INT_MAX);
    TEST_FORMAT(72, "    +2147483647", "%+15i", INT_MAX);
    TEST_FORMAT(73, "    -2147483648", "%+15i", INT_MIN);
    TEST_FORMAT(74, "                                             +2999", "%+50i", 2999);
    TEST_FORMAT(75, "-123", "%+1i", -123);

    // field width and space for sign
    TEST_FORMAT(80, "   27", "% 5i", 27);
    TEST_FORMAT(81, " 2147483647", "% 5i", INT_MAX);
    TEST_FORMAT(82, "     2147483647", "% 15i", INT_MAX);
    TEST_FORMAT(83, "    -2147483648", "% 15i", INT_MIN);
    TEST_FORMAT(84, "                                              2999", "% 50i", 2999);
    TEST_FORMAT(85, "-123", "% 1i", -123);

    // zero padded
    TEST_FORMAT(90, "00027", "%05i", 27);
    TEST_FORMAT(91, "2147483647", "%05i", INT_MAX);
    TEST_FORMAT(92, "000002147483647", "%015i", INT_MAX);
    TEST_FORMAT(93, "-00002147483648", "%015i", INT_MIN);
    TEST_FORMAT(94, "00000000000000000000000000000000000000000000002999", "%050i", 2999);
    TEST_FORMAT(95, "-123", "%01i", -123);

    // zero and left, '-' overrides '0'
    TEST_FORMAT(100, "27   ", "%0-5i", 27);
    TEST_FORMAT(101, "2147483647", "%0-5i", INT_MAX);
    TEST_FORMAT(102, "2147483647     ", "%0-15i", INT_MAX);
    TEST_FORMAT(103, "-2147483648    ", "%0-15i", INT_MIN);
    TEST_FORMAT(104, "2999                                              ", "%0-50i", 2999);
    TEST_FORMAT(105, "-123", "%0-1i", -123);
    TEST_FORMAT(106, "2999                                              ", "%-050i", 2999); // flag order doesn't matter

    // zero padded with plus sign ('0' and '+')
    TEST_FORMAT(110, "+0027", "%+05i", 27);
    TEST_FORMAT(111, "+2147483647", "%+05i", INT_MAX);
    TEST_FORMAT(112, "+00002147483647", "%+015i", INT_MAX);
    TEST_FORMAT(113, "-00002147483648", "%+015i", INT_MIN);
    TEST_FORMAT(114, "+0000000000000000000000000000000000000000000002999", "%+050i", 2999);
    TEST_FORMAT(115, "-123", "%+01i", -123);
    TEST_FORMAT(114, "+0000000000000000000000000000000000000000000002999", "%0+50i", 2999); // flag order doesn't matter

    // precision
    TEST_FORMAT(130, "00027", "%.5i", 27);
    TEST_FORMAT(131, "2147483647", "%.5i", INT_MAX);
    TEST_FORMAT(132, "000002147483647", "%.15i", INT_MAX);
    TEST_FORMAT(133, "-000002147483648", "%.15i", INT_MIN);
    TEST_FORMAT(134, "00000000000000000000000000000000000000000000002999", "%.50i", 2999);

    // precision and '+'
    TEST_FORMAT(140, "+00027", "%+.5i", 27);
    TEST_FORMAT(141, "+2147483647", "%+.5i", INT_MAX);
    TEST_FORMAT(142, "+000002147483647", "%+.15i", INT_MAX);
    TEST_FORMAT(143, "-000002147483648", "%+.15i", INT_MIN);
    TEST_FORMAT(144, "+00000000000000000000000000000000000000000000002999", "%+.50i", 2999);

    // precision and ' '
    TEST_FORMAT(150, " 00027", "% .5i", 27);
    TEST_FORMAT(151, " 2147483647", "% .5i", INT_MAX);
    TEST_FORMAT(152, " 000002147483647", "% .15i", INT_MAX);
    TEST_FORMAT(153, "-000002147483648", "% .15i", INT_MIN);
    TEST_FORMAT(154, " 00000000000000000000000000000000000000000000002999", "% .50i", 2999);

    // field width and precision
    TEST_FORMAT(160, "00027", "%.5i", 27);
    TEST_FORMAT(161, "     00027", "%10.5i", 27);
}
