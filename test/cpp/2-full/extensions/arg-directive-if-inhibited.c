#define FOO 1
#define STRINGIFY(x) # x
STRINGIFY(
        #if 0
        0
        #else
        FOO
        #endif
        )
