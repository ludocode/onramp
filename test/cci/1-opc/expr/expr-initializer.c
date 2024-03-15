int main(void) {
    int x = 5 - 3;
    if (x != 2) {return 1;}

    int y = 5;
    int z = y - y;
    if (z != 0) {return 2;}

    return 0;
}
