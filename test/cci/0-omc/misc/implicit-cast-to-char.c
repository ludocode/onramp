int foo(void) {
    return -1;
}

int main(void) {
    char c = foo();
    if (c != -1) {
        return 1;
    }
    return 0;
}
