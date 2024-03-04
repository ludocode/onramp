int foo(int x) {
    return x;
}

int bar(void) {
    return foo(0);
}

int main(void) {
    return bar();
}
