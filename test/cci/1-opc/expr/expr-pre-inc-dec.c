int main(void) {
    int x = 5;
    if (++x != 6) {return 1;}
    if (x != 6) {return 1;}
    if (--x != 5) {return 1;}
    if (x != 5) {return 1;}
    return 0;
}