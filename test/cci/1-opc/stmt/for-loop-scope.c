int main(void) {
    for (int x = 1; x < 4; x = (x + 1)) {}
    return x; // error, x should not be reachable
}
