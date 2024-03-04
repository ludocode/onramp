#include <stdio.h>

char* word;
char** words;

int main(void) {
    words = &word;
    *words = "Hello";
    puts(word);
    puts(*(words + 0));
}
