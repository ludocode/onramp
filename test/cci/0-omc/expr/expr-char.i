int foo(void) {
    return 'f';
}

int bar(void) {
    return '\n';
}

int main(void) {
    if (foo() != 102) {
        return 1;
    }
    if (bar() == 0x0A) {
        return 0;
    }
    return 1;
}
