int main(void) {
    int count = 5;
    int total = 0;
    do {
        count = (count - 1);
        total = (total + 2);
    } while (count > 0);
    if (total != 10) {
        return 1;
    }
    return 0;
}
