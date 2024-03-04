// should be (1 ? 0 : (0 + 1))
#if 1 ? 0 : 0 + 1
nope
#endif

#if (1 ? 0 : 0) + 1
yup
#endif

// should be (0 ? (1 + 1) : 0)
#if 0 ? 1 + 1 : 0
nope
#endif
