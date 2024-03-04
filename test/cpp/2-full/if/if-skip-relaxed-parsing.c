// C99 spec, section 6.10.2.4
// bug in SCC
#if 0
    #if FOO( * { ; 0123
        ignored
    #endif
#endif
