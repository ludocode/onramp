#define RUN(test) extern void test(void); test()

void __malloc_init(void);

int main(void) {
    __malloc_init();
    RUN(test_stdlib_malloc);
}
