// Most compilers don't bother to implement the comma operator in the
// preprocessor.
// - GCC and LCC parse this correctly.
// - MSVC outputs warnings and nonsense, it ignores everything after the first `,`.
// - TinyCC outputs nothing at all and returns success.
// - Clang, chibicc and Kefir fail with errors on the first comma.
// - SCC outputs nothing, but prints errors only where the left hand side is true.
#if 0, 1
yup
#endif
#if 1, 0
nope
#endif
#if 1, 0, 1, 0, 1
yup
#endif
