// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#define _GNU_SOURCE
#include <stdio.h>

#include <stdlib.h>
#include <string.h>


/*
 * directive parsing
 */

#ifdef DISABLED
static void test_directive_equal(directive_t* actual, directive_t* expected) {
    test_assert(0 == memcmp(actual, expected, sizeof(*actual)));
}

static void test_directive_parse_match(const char* s, directive_t* expected) {
    directive_t actual;
    const char* end = s + strlen(s);
    test_assert(directive_parse(&actual, &s, end));
    test_assert(s == end); // make sure the entire string was consumed
    test_directive_equal(&actual, expected);
}

static void test_stdio_format_directive_parse_s(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.conversion = (uint8_t)'s';
    test_directive_parse_match("%s", &expected);
}

static void test_stdio_format_directive_parse_LS(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.length_modifier = length_modifier_L;
    expected.conversion = (uint8_t)'S';
    test_directive_parse_match("%LS", &expected);
}

static void test_stdio_format_directive_parse_li(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.conversion = (uint8_t)'i';
    expected.length_modifier = length_modifier_l;
    test_directive_parse_match("%li", &expected);
}

static void test_stdio_format_directive_parse_lli(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.conversion = (uint8_t)'i';
    expected.length_modifier = length_modifier_ll;
    test_directive_parse_match("%lli", &expected);
}

static void test_stdio_format_directive_parse_flag_alternate(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.alternate = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%#g", &expected);
}

static void test_stdio_format_directive_parse_flag_zero_padded(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.zero_padded = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%0g", &expected);
}

static void test_stdio_format_directive_parse_flag_left_adjusted(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.left_adjusted = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%-g", &expected);
}

static void test_stdio_format_directive_parse_flag_plus_sign(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.plus_sign = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%+g", &expected);
}

static void test_stdio_format_directive_parse_flag_blank_plus_sign(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.blank_plus_sign = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("% g", &expected);
}

static void test_stdio_format_directive_parse_flag_thousands_grouping(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.thousands_grouping = true;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%'g", &expected);
}

static void test_stdio_format_directive_parse_flag_locale_digits(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.locale_digits = true;
    expected.conversion = (uint8_t)'i';
    test_directive_parse_match("%Ii", &expected);
}

static void test_stdio_format_directive_parse_all_flags(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.alternate = true;
    expected.zero_padded = true;
    expected.left_adjusted = true;
    expected.plus_sign = true;
    expected.blank_plus_sign = true;
    expected.thousands_grouping = true;
    expected.locale_digits = true;
    expected.conversion = (uint8_t)'s';
    test_directive_parse_match("%#0- +'Is", &expected);
}

static void test_stdio_format_directive_parse_position(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.conversion_argument_position = 4;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%4$d", &expected);
}

static void test_stdio_format_directive_parse_position_range(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.conversion_argument_position = 90; // valid digits range from zero to nine
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%90$d", &expected);
}

static void test_stdio_format_directive_parse_field_width(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.field_width = 50;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%50d", &expected);
}

static void test_stdio_format_directive_parse_precision(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.precision = 70;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%.70d", &expected);
}

static void test_stdio_format_directive_parse_field_width_by_argument(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.field_width_as_argument = true;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%*d", &expected);
}

static void test_stdio_format_directive_parse_field_width_by_argument_position(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.field_width_as_argument = true;
    expected.conversion_argument_position = 5;
    expected.field_width = 14;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%5$*14$d", &expected);
}

static void test_stdio_format_directive_parse_precision_by_argument(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.precision_as_argument = true;
    expected.conversion = (uint8_t)'f';
    test_directive_parse_match("%.*f", &expected);
}

static void test_stdio_format_directive_parse_precision_by_argument_position(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.precision_as_argument = true;
    expected.conversion_argument_position = 2;
    expected.precision = 14;
    expected.conversion = (uint8_t)'g';
    test_directive_parse_match("%2$.*14$g", &expected);
}

static void test_stdio_format_directive_parse_field_width_and_precision(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.field_width = 105;
    expected.precision = 99;
    expected.conversion = (uint8_t)'e';
    test_directive_parse_match("%105.99e", &expected);
}

static void test_stdio_format_directive_parse_all_argument_positions(void) {
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.conversion_argument_position = 101;
    expected.field_width = 2;
    expected.precision = 97;
    expected.field_width_as_argument = true;
    expected.precision_as_argument = true;
    expected.conversion = (uint8_t)'d';
    test_directive_parse_match("%101$*2$.*97$d", &expected);
}

static void test_stdio_format_directive_parse_number_max(void) {
#ifdef DISABLED // TODO test max later
    directive_t expected;
    memset(&expected, 0, sizeof(expected));
    expected.argument_positions = true;
    expected.conversion_argument_position = NUMBER_MAX;
    expected.conversion = (uint8_t)'p';

    // TODO don't use snprintf here!
    char buf[100];
    snprintf(buf, sizeof(buf), "%%%" PRIu64 "$p", (uint64_t)NUMBER_MAX);

    test_directive_parse_match(buf, &expected);
#endif
}
#endif



/*
 * format printing
 */

static void test_stdio_format_printf() {
    printf("Hello\n");
    printf("Hello %s!\n", "world");
    printf("%u\n", 0);
    printf("%u\n", 10);
    printf("%i\n", 10);
    printf("%i\n", -10);
    printf("%d %d %d %d\n", 1, 2, 3, 4);

    printf("%.3d\n", 33);


    // another bug at one point
    char* f;
    if (0 > asprintf(&f, "%s.od", "foo")) {_Exit(1);}
    if (0 != strcmp(f, "foo.od")) {_Exit(1);}
    free(f);
}


int main(void) {

#ifdef DISABLED
    // directive parsing
    test_stdio_format_directive_parse_s();
    test_stdio_format_directive_parse_LS();
    test_stdio_format_directive_parse_li();
    test_stdio_format_directive_parse_lli();
    test_stdio_format_directive_parse_flag_alternate();
    test_stdio_format_directive_parse_flag_zero_padded();
    test_stdio_format_directive_parse_flag_left_adjusted();
    test_stdio_format_directive_parse_flag_plus_sign();
    test_stdio_format_directive_parse_flag_blank_plus_sign();
    test_stdio_format_directive_parse_flag_thousands_grouping();
    test_stdio_format_directive_parse_flag_locale_digits();
    test_stdio_format_directive_parse_all_flags();
    test_stdio_format_directive_parse_position();
    test_stdio_format_directive_parse_position_range();
    test_stdio_format_directive_parse_field_width();
    test_stdio_format_directive_parse_precision();
    test_stdio_format_directive_parse_field_width_by_argument();
    test_stdio_format_directive_parse_field_width_by_argument_position();
    test_stdio_format_directive_parse_precision_by_argument();
    test_stdio_format_directive_parse_precision_by_argument_position();
    test_stdio_format_directive_parse_field_width_and_precision();
    test_stdio_format_directive_parse_all_argument_positions();
    test_stdio_format_directive_parse_number_max();
#endif

    // format printing
    test_stdio_format_printf();
}
