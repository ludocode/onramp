struct foo {
    int a;
    int b;
    int c;
};

int main(void) {
    if (sizeof(struct foo) != 12) {
        return 1;
    }

    struct foo f;
    if (sizeof(f) != 12) {
        return 1;
    }

    return 0;
}
