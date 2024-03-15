int main(void) {
    if (sizeof(-1) != 4) {
        return 1;
    }
    if (sizeof((char)1) != 1) {
        return 1;
    }
    if (sizeof(2 * 3) != 4) {
        return 1;
    }
    if (sizeof((char)1 / (char)2) != 4) {
        return 1;
    }
    void** p;
    if (sizeof(p + 1) != 4) {
        return 1;
    }

    // sizeof without parens has high precedence
    if (sizeof 1 - sizeof(int)) {
        return 1;
    }

    return 0;
}
