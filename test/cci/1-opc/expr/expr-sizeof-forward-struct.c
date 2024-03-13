struct foo;

int main(void) {
    return sizeof(struct foo); // ERROR, forward-declared struct has no size
}
