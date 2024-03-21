struct {
    int a;
    union {
        int ba[2];
        struct {
            int bba;
            int bbb;
        } bb;
        int bc;
    } b[4];
    int c[8];
} foo[10];

int main(void) {
    if (sizeof(foo) != 680) {
        return 1;
    }
    return 0;
}
