int main(void) {
    if (2 >= 3) {
        return 1;
    }
    if (3 >= 3) {
        if (4 >= 3) {
            return 0;
        }
    }
    return 1;
}
