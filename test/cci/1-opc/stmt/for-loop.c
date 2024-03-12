int main(void) {
    int x = 0;

    // for loop with declaration
    for (int y = 0; y < 5; y = (y + 1)) {
        x = (x + y);
    }
    if (x != 10) {
        return 1;
    }

    // for loop without declaration, comma operator in expression
    int y;
    for (y = 5, y = 3; y > -2; y = (y - 1)) {
        x = (x - 1);
    }
    if (x != 5) {
        return 1;
    }

    // for loop with false condition
    for (x = 1; false; x = 10) {
        return 1;
    }
    if (x != 1) {
        return 1;
    }

    // infinite for loop
    x = 1;
    for (;;) {
        x = 2;
        break;
    }
    if (x != 2) {
        return 1;
    }

    return 0;
}
