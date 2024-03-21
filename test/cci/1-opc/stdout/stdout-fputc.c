// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

extern int fputc(int c, void* file);
extern void* stdout;

int main(void) {
    fputc(0x61, stdout);
    return 0;
}
