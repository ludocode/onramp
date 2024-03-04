/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Generates all solutions to the 8 queens problem, printing them to standard
 * output.
 *
 * TODO replace this with cci/2-full
 */

#include <stdio.h>
#include <stdlib.h>

char* queens;
void place_queen(int x);
static void print_board(void);

int main(void) {
    queens = malloc(8);
    place_queen(0);
    free(queens);
}

int is_queen_valid(int x, int y) {
    int i;
    i = 0;
    while (i < x) {
        int q;
        int diff;
        q = *(queens + i);
        diff = (x - i);
        if (((q == y) | (q == (y - diff))) | (q == (y + diff))) {
            return 0;
        }
        i = (i + 1);
    }
    return 1;
}

void place_queen(int x) {
    if (x == 8) {
        print_board();
        return;
    }

    int y;
    y = 0;
    while (y < 8) {
        if (is_queen_valid(x, y)) {
            *(queens + x) = y;
            place_queen(x + 1);
        }
        y = (y + 1);
    }
}

void print_board(void) {
    int y;
    y = 0;
    while (y < 8) {
        int x;
        x = 0;
        while (x < 8) {
            int match;
            match = (*(queens + x) == y);
            if (match) {
                putchar('Q');
            }
            if (!match) {
                putchar('.');
            }
            putchar(' ');
            x = (x + 1);
        }
        putchar('\n');
        y = (y + 1);
    }
    putchar('\n');
    putchar('\n');
}
