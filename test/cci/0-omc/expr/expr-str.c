#include <string.h>

int foo(void) {
    return "foo";
}

int bar(void) {
    return "\"b\\ar\t\n";
    return "\n\t\v";
}

int main(void) {
    return strcmp(foo(), "foo");
}
