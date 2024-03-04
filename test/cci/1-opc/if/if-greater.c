#include <limits.h>

int main(void) {
    if (1 > 5) {
        return 1;
    }
    if (-1 > 5) {
        return 1;
    }
    if (-5 > 1) {
        return 1;
    }
    if (-10 > -5) {
        return 1;
    }
    if (INT_MIN > (INT_MIN+1)) {
        return 1;
    }
    if ((INT_MAX-1) > INT_MAX) {
        return 1;
    }
    if (INT_MIN > INT_MAX) {
        return 1;
    }

    // equal is false
    if (1 > 1) {
        return 1;
    }
    if (INT_MIN > INT_MIN) {
        return 1;
    }
    if (INT_MAX > INT_MAX) {
        return 1;
    }
    return 0;
}
