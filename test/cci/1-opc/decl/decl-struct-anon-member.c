struct foo {
    union {
        int x;
    }; // ERROR, opC doesn't support anonymous structs and unions
} foo;

int main(void) {
    foo.x; // this caused a compiler crash at one point
    return 0;
}
