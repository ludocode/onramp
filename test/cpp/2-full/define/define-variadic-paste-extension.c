#define FOO(...) before,##__VA_ARGS__,after
FOO()
FOO(1)
FOO(1,2)
FOO(1,2,3)
FOO(1,(2,3),4)
