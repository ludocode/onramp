int main(void) {

    // simple label test
    goto foo;
    return 1;
foo:

    // jumping into a loop
    int x = 0;
    goto bar;
    return 1;
    while (1) {
        break;
    bar:
        x = 1;
    }
    if (x != 1) {
        return 1;
    }

    // unbraced if statement with label
    if (0)
        baz: return 1;

    return 0;
}
