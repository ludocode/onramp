typedef struct {
    int a;
    int b;
    int c;
} foo_t;

int main(void) {
    if (sizeof(foo_t) != 12) {
        return 1;
    }
    return 0;
}
