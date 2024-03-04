// TinyCC accepts this.
// chibicc outputs no error message but exits with an error.
// GCC, Clang, MSVC and Kefir correctly diagnose the error.
#define FOO() "include-hello.h"
#include FOO
            ()
