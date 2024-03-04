#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

// TODO rename to __fatal
_Noreturn void __libc_error(const char* message);

#include <assert.h>

// TODO message
#define libc_assert(expression, ...) \
    ((expression) ? ((void)0) : \
        __assert_fail(#expression, __FILE__, __LINE__, __func__))

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define CONST_CAST(T, expr) ((T)(expr))

#endif
