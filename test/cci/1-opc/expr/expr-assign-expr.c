int x;
int main(void) {
    x = 4 + 7;
    if (x != 11) {
        return 1;
    }

    int y = 4;
    x = y = 1;
    if (x != 1) {
        return 1;
    }
    if (y != 1) {
        return 1;
    }

    return 0;
}
