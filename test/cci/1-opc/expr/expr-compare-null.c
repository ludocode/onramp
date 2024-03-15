int main(void) {
    int* x = 0;

    // making sure that we're allowed to compare a pointer with a literal zero
    // (comparisons between pointers and ints are not allowed otherwise)
    if (x != 0) {
        return 1;
    }
    if (x == 0) {
        return 0;
    }
    return 1;
}
