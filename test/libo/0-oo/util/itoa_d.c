#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

void __itoa_d(int value, char* buffer);

char* buf;

int main(void) {
    buf = malloc(12);

    __itoa_d(0, buf);
    puts(buf);

    __itoa_d(1, buf);
    puts(buf);

    __itoa_d(9, buf);
    puts(buf);

    __itoa_d(15, buf);
    puts(buf);

    __itoa_d(1234567, buf);
    puts(buf);

    __itoa_d(INT_MAX, buf);
    puts(buf);

    __itoa_d(-1, buf);
    puts(buf);

    __itoa_d(-2, buf);
    puts(buf);

    __itoa_d(-9, buf);
    puts(buf);

    __itoa_d(-99, buf);
    puts(buf);

    __itoa_d(-1234567, buf);
    puts(buf);

    __itoa_d(INT_MIN, buf);
    puts(buf);
}
