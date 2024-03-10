int main(void) {
    int x[3 + 8 * sizeof(short) / 3];
    if (sizeof(x) != 32) {
        return 1;
    }
    return 0;
}
