#include <limits.h>
#include <stdio.h>

int main(void) {

    // all combinations of signs
    if ((19 % 7) != 5) {
        return 1;
    }
    if ((-19 % 7) != -5) {
        return 1;
    }
    if ((19 % -7) != 5) {
        return 1;
    }
    if ((-19 % -7) != -5) {
        return 1;
    }

    // modulo of INT_MIN (magnitude not representable as signed int)
    if ((INT_MIN % 7919) != -1309) {
        return 1;
    }

    return 0;
}
