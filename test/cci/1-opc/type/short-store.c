int main(void) {
    short x = 0xfff2345;
    if (x != 0x2345) {
        return 1;
    }
    return 0;
}
