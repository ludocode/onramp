// This fails under chibicc.
// It works under GCC, Clang, MSVC, TinyCC and Kefir.
#define FOO
#include "include-hello.h" FOO
