// Undefined behaviour according to C99.
// Clang warns by default, GCC warns under -Wextra, MSVC warns only under the
// new conforming preprocessor, Kefir does not warn. All of them produce the
// expected result (false).
// chibicc and TinyCC diagnose errors.
#define FOO defined
#if FOO(X)
x
#endif
