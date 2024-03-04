#include "stdio_file.c"

#include "test.h"

static void test_stdio_file_stdout(void) {
    fwrite("Hello\n", 1, 6, stdout);
}

void test_stdio_file() {
    test_stdio_file_stdout();
    puts("stdio file tests pass");
}
