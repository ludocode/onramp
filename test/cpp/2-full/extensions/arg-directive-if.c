#define FOO(x) x
FOO(
    #if 0
    0
    #else
    1
    #endif
)
