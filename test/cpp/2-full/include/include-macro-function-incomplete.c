/* MSVC parses this as though it had a `)`. */
/* TinyCC gives the wrong error message ("too many arguments") but does at least diagnose it. */
/* GCC, Clang, chibicc and kefir correctly diagnose the error. */
#define FOO() "include-hello.h"
#include FOO(
