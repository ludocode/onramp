#include <stdio.h>
#include <stdlib.h>

// a terrible number formatting routine

static char* write_number_buffer;
static int write_number_buffer_capacity;

static void write_number(FILE* file, int number) {
    if (number == 0) {
        fwrite("0", 1, 1, file);
        return;
    }

    char* buffer;
    buffer = write_number_buffer;
    int i;
    i = (write_number_buffer_capacity - 1);

    while (number > 0) {
        i = (i - 1);
        int digit;
        digit = (number - ((number / 10) * 10)); // modulus not implemented yet
        *(buffer + i) = ('0' + digit);
        number = (number / 10);
    }

    size_t len;
    len = (write_number_buffer_capacity - (i + 1));
    fwrite(buffer + i, 1, len, file);
}

int main(void) {
    write_number_buffer_capacity = 32;
    write_number_buffer = malloc(write_number_buffer_capacity);
    write_number(stdout, 54081);
    putchar('\n');
}
