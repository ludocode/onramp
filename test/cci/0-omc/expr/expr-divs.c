#include <limits.h>

int main(void) {

    // both positive
    if ((7 / 3) != 2) {
        return 1;
    }

    // positive dividend, negative divisor
    if ((INT_MAX / -31) != -69273666) {
        return 2;
    }

    // negative dividend, positive divisor
    if ((-719 / 67) != -10) {
        return 3;
    }

    // both negative
    if ((-600 / -20) != 30) {
        return 4;
    }

    char x = 4;
    int y = 1;
    if ((x / y) != x) {
        return 5;
    }

    // division of INT_MIN (magnitude not representable as signed int)
    if ((INT_MIN / 1) != INT_MIN) {
        return 6;
    }

    return 0;
}
