int main(void) {
    // a bug building ld/1
    int offset = 12;
    if ((offset < -0x8000) | (offset > 0xFFFF)) {
        return 1;
    }
    return 0;
}
