// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdio.h>
#include <string.h>

typedef struct {
    int x, y;
} point_t;

int main(void) {
    point_t points[3];
    memset(points, 0, sizeof(points));

    point_t* point = &points[0];

    // A bug in Onramp was causing the left hand size of a compound expression
    // to be evaluated twice. It should be evaluated exactly once.
    point++->x |= 1;

    /*
    printf("%i\n",points[0].x);
    printf("%i\n",points[0].y);
    printf("%i\n",points[1].x);
    printf("%i\n",points[1].y);
    printf("%i\n",points[2].x);
    printf("%i\n",points[2].y);
    */

    if (point != &points[1]) return 1;

    if (points[0].x != 1) return 2;
    if (points[0].y != 0) return 3;
    if (points[1].x != 0) return 4;
    if (points[1].y != 0) return 5;
    if (points[2].x != 0) return 6;
    if (points[2].y != 0) return 7;
}
