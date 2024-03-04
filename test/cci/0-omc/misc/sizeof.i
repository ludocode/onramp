int main(void) {
    if (sizeof(int) != 4) {
        return 1;
    }
    if (sizeof(char) != 1) {
        return 1;
    }
    if (sizeof(void) != 1) {
        return 1;
    }
    if (sizeof(char*) != 4) {
        return 1;
    }
    if (sizeof(void******) != 4) {
        return 1;
    }
    return 0;
}
