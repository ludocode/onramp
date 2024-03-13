#include <string.h>

int main(void) {
    char x[6];
    x[0] = 'H';
    x[1] = 'e';
    x[2] = 'l';
    x[3] = 'l';
    x[4] = 'o';
    x[5] = 0;
    if (0 != strcmp(x, "Hello")) {
        return 1;
    }
    return 0;
}
