// The MIT License (MIT)
// Copyright (c) 2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// dummy types that don't match anything
typedef struct {int unused;} n1;
typedef struct {int unused;} n2;
typedef struct {int unused;} n3;
typedef struct {int unused;} n4;

int main(void) {

    // Test types for numeric literals.
    // This is testing the table in C17 6.4.4.1.5 .

    // (Note: For numbers larger than INT_MAX or UINT_MAX we allow `long` or
    // `long long` because we don't know if `long` is 64 bits on this platform.
    // When testing with Onramp, `long` is always 32 bits.)

    // TODO enable this test. We need _Generic in cci/2 and function-like
    // macros and __COUNTER__ in cpp/2. In the meantime these have been checked
    // manually by looking at the parse tree.

    // checks that the expression has one of the given types
    #define CHECK_TYPE(expr, ...) CHECK_TYPE_IMPL(expr, __VA_ARGS__, n1, n2, n3, n4)
    #define CHECK_TYPE_IMPL(expr, a, b, c, d, ...) \
        _Generic((expr), \
                a: (void)0, \
                b: (void)0, \
                c: (void)0, \
                d: (void)0, \
                default: ({return __COUNTER__;}))

    // make sure suffixes are working properly
    CHECK_TYPE(1, int);
    CHECK_TYPE(1l, long);
    CHECK_TYPE(1ll, long long);
    CHECK_TYPE(1u, unsigned);
    CHECK_TYPE(1ul, unsigned long);
    CHECK_TYPE(1ull, unsigned long long);
    CHECK_TYPE(0x1, int);
    CHECK_TYPE(0x1l, long);
    CHECK_TYPE(0x1ll, long long);
    CHECK_TYPE(0x1u, unsigned);
    CHECK_TYPE(0x1ul, unsigned long);
    CHECK_TYPE(0x1ull, unsigned long long);
    CHECK_TYPE(01, int);
    CHECK_TYPE(01l, long);
    CHECK_TYPE(01ll, long long);
    CHECK_TYPE(01u, unsigned);
    CHECK_TYPE(01ul, unsigned long);
    CHECK_TYPE(01ull, unsigned long long);

    // make sure unsigned int can go up to UINT32_MAX
    CHECK_TYPE(2147483648u, unsigned); // INT32_MAX + 1
    CHECK_TYPE(4294967295u, unsigned); // UINT32_MAX
    CHECK_TYPE(4294967295u, unsigned); // UINT32_MAX
    CHECK_TYPE(4294967296u, unsigned long long); // UINT32_MAX + 1

    // make sure decimal numbers out of range get upgraded to 64-bit and are
    // always signed
    CHECK_TYPE(2147483648, long, long long); // INT32_MAX + 1
    CHECK_TYPE(4294967295, long, long long); // UINT32_MAX
    CHECK_TYPE(9223372036854775807, long, long long); // INT64_MAX

    // Note: The implementation is allowed to upgrade this to unsigned but it
    // is optional. GCC and Clang both do it and warn about it so Onramp does
    // it too. If the implementation can't upgrade it, the spec says the
    // integer constant "has no type". I don't know what that means.
    CHECK_TYPE(18446744073709551615, unsigned long, unsigned long long); // UINT64_MAX

    // make sure hex, octal and binary numbers out of range get upgraded to
    // unsigned if they fit, otherwise 64-bit signed if they fit, otherwise
    // 64-bit unsigned.
    // TODO uncomment the binary literals, we need to add binary literal parsing to cci/2
    CHECK_TYPE(0x7fffffff, int); // INT32_MAX
    CHECK_TYPE(017777777777, int); // INT32_MAX
    CHECK_TYPE(0b111'1111'1111'1111'1111'1111'1111'1111, int); // INT32_MAX
    CHECK_TYPE(0x80000000, unsigned); // INT32_MAX + 1
    CHECK_TYPE(020000000000, unsigned); // INT32_MAX + 1
    CHECK_TYPE(0b1000'0000'0000'0000'0000'0000'0000'0000, unsigned); // UINT32_MAX
    CHECK_TYPE(0xFFFFFFFF, unsigned); // UINT32_MAX
    CHECK_TYPE(037777777777, unsigned); // UINT32_MAX
    CHECK_TYPE(0b1111'1111'1111'1111'1111'1111'1111'1111, unsigned); // UINT32_MAX
    CHECK_TYPE(0x100000000, long, long long); // UINT32_MAX + 1
    CHECK_TYPE(040000000000, long, long long); // UINT32_MAX + 1
    CHECK_TYPE(0b1'0000'0000'0000'0000'0000'0000'0000'0000, long, long long); // UINT32_MAX + 1
    CHECK_TYPE(0x8000000000000000, unsigned long, unsigned long long); // INT64_MAX + 1
    CHECK_TYPE(01000000000000000000000, unsigned long, unsigned long long); // INT64_MAX + 1
    CHECK_TYPE(0b1000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000, unsigned long, unsigned long long); // INT64_MAX + 1
    CHECK_TYPE(0xffffffffffffffff, unsigned long, unsigned long long); // UINT64_MAX
    CHECK_TYPE(01777777777777777777777, unsigned long, unsigned long long); // UINT64_MAX
    CHECK_TYPE(0b1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111, unsigned long, unsigned long long); // UINT64_MAX
}
