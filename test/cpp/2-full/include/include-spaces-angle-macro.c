// This fails in all compilers I've tested:
// GCC, Clang, MSVC, TinyCC, chibicc, Kefir all fail.
#define FOO <include  spaces.h>
#include FOO
