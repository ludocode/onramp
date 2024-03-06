int main(void) {
    if ((-1 + -10) != -11) {
        return 1;
    }

    if ((5 + 3) != 8) {
        return 1;
    }

    // test promotion from char to int
    char x = -3;
    if ((x + -5) != -8) {
        return 1;
    }

    return 0;
}
