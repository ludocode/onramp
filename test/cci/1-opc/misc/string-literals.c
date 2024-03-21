// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

extern int putchar(int c);
extern int puts(const char* s);
extern int fputs(const char* s, void* file);
extern void* stdout;

int main(void) {
    fputs("Hello", stdout);
    putchar(' ');
    puts("world!");
    return 0;
}

void foo(void) {
}
