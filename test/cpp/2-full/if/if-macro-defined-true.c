// Undefined behaviour according to C99.
// Clang warns by default, GCC warns under -Wextra, MSVC warns only under the
// new conforming preprocessor, Kefir does not warn. All of them produce the
// expected result (true).
// chibicc and TinyCC diagnose errors.
#define FOO defined
#define X
#if FOO(X)
x
#endif
