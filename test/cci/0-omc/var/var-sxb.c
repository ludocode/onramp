int main(void) {
    char x = -1;
    // This assignment should include a cast from char to int which should
    // trigger sign extension of the char (otherwise y will be 255.)
    int y = x;
    if (y != -1) {
        return 1;
    }
    return 0;
}
