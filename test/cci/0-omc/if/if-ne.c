int main(void) {
    if (3 != 3) {
        return 1;
    }
    if (2 != 3) {
        if (4 != 3) {
            return 0;
        }
    }
    return 0;
}
