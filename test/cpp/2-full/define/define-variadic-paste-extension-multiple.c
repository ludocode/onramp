#define FOO(x,...) before,x,##__VA_ARGS__,after
FOO(1)
FOO(1,2)
FOO(1,2,3)
FOO(1,(2,3),4)
