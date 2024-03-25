// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

struct {
    int a;
    int b : 3;
    char c : 5;
    int : 9;
    short d : 7;
} foo;

int main(void) {

    foo.a = 1;
    foo.b = 2;
    foo.c = 3;
    foo.d = 4;

    if (foo.a != 1) {return 1;}
    if (foo.b != 2) {return 2;}
    if (foo.c != 3) {return 3;}
    if (foo.d != 4) {return 4;}

    return 0;
}
