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

#include "common.h"

bool line_manual;

int current_char;
int current_address;
int file_start_address;
int file_index;
bool optimize;

FILE* output_file;
FILE* input_file;
FILE* debug_file;
fpos_t file_start_pos;
int pass;
char file_first_char;

char* buffer;
size_t buffer_length;

struct symbol_t* current_symbol;

bool option_optimize = false;
bool option_debug = false;

// TODO this should be in libo
int hex_to_int(char c) {
    if ((c >= '0') & (c <= '9')) {
        return c - '0';
    }
    if ((c >= 'a') & (c <= 'f')) {
        return (c - 'a') + 10;
    }
    if ((c >= 'A') & (c <= 'F')) {
        return (c - 'A') + 10;
    }
    fatal("Expected hexadecimal character, not 0x%X", c);
}

// TODO this should also be in libo
bool is_end_of_line(int c) {
    return (c == '\n') || (c == '\r') || (c == -1);
}
