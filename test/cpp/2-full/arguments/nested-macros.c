#define FOO(x) BAR(BAZ(x))
#define BAR(y) 1 y -1
#define BAZ(z) 2 z -2
FOO(w)
