int main(void) {
    int x, y;
    // Error, the left-hand side of an assignment must be a unary expression
    // and an lvalue. (TinyCC incorrectly compiles this without error.)
    1 ? x : y = 4;
    return 1;
}
