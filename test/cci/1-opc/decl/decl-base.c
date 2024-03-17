int main(void) {
    if (sizeof(char) != 1) {return 1;}
    if (sizeof(signed char) != 1) {return 1;}
    if (sizeof(unsigned char) != 1) {return 1;}

    if (sizeof(unsigned short) != 2) {return 1;}

    if (sizeof(unsigned int) != 4) {return 1;}
    if (sizeof(unsigned) != 4) {return 1;}

    if (sizeof(unsigned long) != 4) {return 1;}

    if (sizeof(signed short) != 2) {return 1;}
    if (sizeof(short) != 2) {return 1;}

    if (sizeof(int) != 4) {return 1;}
    if (sizeof(signed) != 4) {return 1;}
    if (sizeof(signed int) != 4) {return 1;}

    if (sizeof(signed long) != 4) {return 1;}
    if (sizeof(long) != 4) {return 1;}

    return 0;
}
