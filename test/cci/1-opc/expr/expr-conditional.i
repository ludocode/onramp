int main(void) {
    int x = 5;
    if (x != 5 ? 4 - 3 : 6 - 6) {
        return 1;
    }
    if (x != 5 ? 4 - 4 : 1 - 0) {
        return 0;
    }
    return 1;
}
