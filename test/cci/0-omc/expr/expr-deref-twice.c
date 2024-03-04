#include <stdio.h>

int main(void) {
    char* s = "Hello";
    char** p = &s;
    putchar(**p);
}
