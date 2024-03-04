// necessary to make ghost_has() work
// bug in SCC
#define A(x) B_##x
#define B_1 1
#if A(1)
    a
#endif
