int main(void) {
    if (-5 != (0 - 5)) {
        return 1;
    }

    int x = -4;
    if (-x != 4) {
        return 1;
    }

    return 0;
}
