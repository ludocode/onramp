int main(void) {
    if (sizeof(int) != 4) {
        return 1;
    }
    if (sizeof(char) != 1) {
        return 2;
    }
    if (sizeof(void) != 1) {
        return 3;
    }
    if (sizeof(char*) != 4) {
        return 4;
    }
    if (sizeof(void******) != 4) {
        return 5;
    }
    return 0;
}
