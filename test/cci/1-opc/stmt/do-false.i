int main(void) {
    int count = 0;
    do count = (count + 1); while (0);
    if (count != 1) {
        return 1;
    }
    return 0;
}
