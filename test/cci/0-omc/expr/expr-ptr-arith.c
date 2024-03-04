int main(void) {
    char* h = (char*)"Hello world!";

    // add scalar on right
    if (*(h + 4) != 'o') {
        return 1;
    }

    // add scalar on left
    if (*(8 + h) != 'r') {
        return 2;
    }

    char* o = (h + 4);
    char* r = (h + 8);

    // positive difference of word pointers
    if (((int*)o - (int*)h) != 1) {
        return 3;
    }

    // negative difference of word pointers
    if (((int*)h - (int*)r) != -2) {
        return 4;
    }

    return 0;
}
