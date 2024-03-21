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
 */

#include <stdio.h>
#include <stdbool.h>

static char queens[8];
static void place_queen(int x);
static void print_board(void);

int main(void) {
    place_queen(0);
}

/**
 * Returns true if the queen in the given row and column would be valid given
 * the queens in the preceding columns, or false otherwise
 */
static bool is_queen_valid(int x, int y) {
    for (int i = 0; i < x; ++i) {
        int q = queens[i];
        int diff = x - i;
        if (q == y || q == y - diff || q == y + diff)
            return false;
    }
    return true;
}

/**
 * Places a queen at all valid positions in the given column, recursing for each
 * to place the next column.
 *
 * If the column number is 8, this instead prints the board.
 */
static void place_queen(int x) {
    if (x == 8) {
        print_board();
        return;
    }
    for (int y = 0; y < 8; ++y) {
        if (is_queen_valid(x, y)) {
            queens[x] = (char)y;
            place_queen(x + 1);
        }
    }
}

/**
 * Prints the board.
 */
static void print_board(void) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            putchar(queens[x] == y ? 'Q' : '.');
            putchar(' ');
        }
        putchar('\n');
    }
    puts("\n");
}
