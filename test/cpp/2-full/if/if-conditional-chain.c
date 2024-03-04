#if 0 ? 0 : 0 ? 0 : 1
// MSVC fails this, both traditional and new. It's not clear to
// me why it fails because it should be 0 regardless of associativity.
yup
#endif

// right-associative
#if 1 ? 1 : 0 ? 0 : 1
yup
#endif
#if 1 ? 1 : (0 ? 0 : 1)
yup
#endif
#if (1 ? 1 : 0) ? 0 : 1
nope
#endif

#if 1 ? 0 : 0 ? 0 : 1
nope
#endif
#if 0 ? 0 : 1 ? 0 : 1
// Same as above, MSVC fails this.
nope
#endif
