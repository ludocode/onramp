int main(void) {

    // == has lower precedence than +
    if (0 == 2 + 1) {
        return 1;
    }

    // simple arithmetic with comparison in the middle
    if (5 * 3 + 10 / 2 != 30 * 2 / 3) {
        return 1;
    }

    return 0;
}
