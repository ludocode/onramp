int main(void) {
    int x = 5;
    int y = 10;
    if ((x * 2) != y) {
        return 1;
    }

    int z = 15;
    if (z != (x + y)) {
        return 1;
    }

    int* px = &x;
    int** ppx = &px;
    int* pz = &z;
    if (px != &x) {
        return 1;
    }
    if (*ppx != &x) {
        return 1;
    }
    if (*pz != (3 * (*(*ppx)))) { // TODO fix extra parens
        return 1;
    }

    int X = *px;
    if (x != X) {
        return 1;
    }

    return 0;
}
