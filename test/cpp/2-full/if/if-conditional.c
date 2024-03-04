#define ZERO 0
#define ONE 1
#if ONE ? ZERO : ONE
nope
#endif
#if ZERO ? ZERO : ONE
yup
#endif
