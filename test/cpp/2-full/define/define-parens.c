#define LP (
#define RP )
LP
RP
#define FOO(x) x
#define EXPAND(x) x
FOO LP 1 RP
EXPAND(FOO LP 1 RP)
