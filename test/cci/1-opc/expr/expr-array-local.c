int main(void) {
    int xa;
    int xb[5];
    int xc;

    xa = -1;
    xc = 5;
    *xb = 0;
    *(xb + 1) = 1;
    *(xb + 2) = 2;
    *(3 + xb) = 3;
    *(4 + xb) = 4;

    if (xa != -1) {return 1;}
    if (xc != 5) {return 1;}
    if (*xb != 0) {return 1;}
    if (*(xb + 1) != 1) {return 1;}
    if (*(2 + xb) != 2) {return 1;}
    if (*(xb + 3) != 3) {return 1;}
    if (*(4 + xb) != 4) {return 1;}

    return 0;
}
