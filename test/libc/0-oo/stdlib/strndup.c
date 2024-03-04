#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {

    // actual bug from strndup() at one point
    char* s = strndup("./include/include-two.c", 9);
    puts(s);
    if (0 != strcmp(s, "./include")) {
        return 1;
    }
    free(s);

    return 0;
}
