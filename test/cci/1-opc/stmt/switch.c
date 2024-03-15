int main(void) {

    // typical switch
    int x = 4;
    switch (x * 2) {
        case 10: return 1;
        default: return 1;
        case 19: return 1;
        case 2*2+2*2: break;
        return 1;
    }

    // switch with no default and no match
    switch ('x') {
        case 'a': return 1;
        case 'b': return 1;
        case 'c': return 1;
    }

    // switch fallthrough
    switch (x = 0, 'b') {
        case 'a': return 1;
        case 'b': x = (x + 1);
        case 'c': x = (x + 1);
        default: x = (x + 1);
    }
    if (x != 3) {
        return 1;
    }

    // switch without braces, match
    x = 1;
    switch (0)
        case 0:
            x = 0;
    if (x) {return 1;}

    // switch without braces, no match
    switch (0)
        case 1:
            return 1;

    // nested switch
    switch (0)
        switch (0)
            default: return 1;

    return 0;
}
