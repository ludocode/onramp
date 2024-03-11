enum foo {
    alice,
    bob,
    carol,
    dave = -10,
    eve = -2,
    fred,
    gary,
    helen,
    ida = 2147483647
};

int main(void) {
    if (alice != 0) {return 1;}
    if (bob != 1) {return 1;}
    if (carol != 2) {return 1;}
    if (dave != -10) {return 1;}
    if (eve != -2) {return 1;}
    if (fred != -1) {return 1;}
    if (gary != 0) {return 1;}
    if (helen != 1) {return 1;}
    if (ida != 2147483647) {return 1;}
    if (gary != alice) {return 1;}
    if (helen != bob) {return 1;}
    return 0;
}
