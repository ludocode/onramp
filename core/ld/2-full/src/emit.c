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

#include "emit.h"

char* emit_filename;
char* emit_current_symbol;
int emit_line = 1;
static int bytes_emitted;

void emit_byte_count(void) {
    if (bytes_emitted == 0) {
        return;
    }
    if (option_debug && emit_filename) {
        fprintf(debug_file, "%i\n", bytes_emitted);
    }
    bytes_emitted = 0;
}

void emit_init(void) {
    // nothing
}

void emit_destroy(void) {
    emit_byte_count();
    free(emit_filename);
    free(emit_current_symbol);
}

void emit_byte(char c) {
    //printf("emit byte '%x pass %i\n", c, pass);
    if (pass == 3) {
        fputc(c, output_file);
        ++bytes_emitted;
    }
}

void emit_short(int s) {
    emit_byte((char)s);
    emit_byte((char)(s >> 8));
}

void emit_int(int s) {
    emit_byte((char)s);
    emit_byte((char)(s >> 8));
    emit_byte((char)(s >> 16));
    emit_byte((char)(s >> 24));
}

void emit_debug(char c) {
    if (pass != 3) {
        return;
    }
    if (!option_debug) {
        return;
    }
    fputc(c, debug_file);
}

void emit_source_location(const char* /*nullable*/ filename, int line) {
    //printf("set source %s %i %i\n",filename,line, pass);
    if (pass != 3) {
        return;
    }
    if (!option_debug) {
        return;
    }

    // TODO if neither have changed, do nothing (e.g. when starting a file or an archive member)

    // shortcut for a single line directive
    if (emit_filename != NULL && // we have previous debug info
            (filename == NULL || 0 == strcmp(filename, emit_filename)) && // filename hasn't changed
            line == emit_line + 1) // line has incremented by one
    {
        emit_byte_count();
        emit_line = line;
        fputs("#\n", debug_file);
        return;
    }

    // TODO emit a #line without the filename if it hasn't changed

    // set the new source location
    if (filename != NULL) {
        free(emit_filename);
        emit_filename = strdup(filename);
    }
    emit_line = line;

    // output a line directive
    emit_byte_count();
    emit_line = line;
    if (emit_filename != NULL) {
        fprintf(debug_file, "#line %i \"%s\"\n", emit_line, emit_filename);
    }
}

void emit_symbol(const char* symbol) {
    if (pass != 3) {
        return;
    }
    if (!option_debug) {
        return;
    }

    // if the symbol hasn't changed, do nothing
    if (emit_current_symbol != NULL && 0 == strcmp(symbol, emit_current_symbol)) {
        return;
    }

    // update the symbol
    emit_byte_count();
    free(emit_current_symbol);
    emit_current_symbol = strdup(symbol);
    fprintf(debug_file, "#symbol %s\n", emit_current_symbol);
}

void emit_increment_line(int line) {
    emit_source_location(NULL, emit_line + 1);
}
