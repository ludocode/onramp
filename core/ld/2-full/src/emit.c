/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

#include "symbol.h"

char* emit_filename;
string_t* emit_current_symbol;
int emit_line;
static int bytes_emitted;

void emit_byte_count(void) {
    if (bytes_emitted == 0) {
        return;
    }
    if (option_debug) {
        fprintf(debug_file, "%i\n", bytes_emitted);
    }
    bytes_emitted = 0;
}

void emit_init(void) {
    emit_line = 1;
}

void emit_destroy(void) {
    emit_byte_count();
    free(emit_filename);
    if (emit_current_symbol) {
        string_deref(emit_current_symbol);
    }
}

void emit_byte(char c) {
    if (output_pass) {
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
    if (!output_pass) {
        return;
    }
    if (!option_debug) {
        return;
    }
    if (current_symbol && !current_symbol->is_used) {
        return;
    }
    fputc(c, debug_file);
}

// TODO this function, and in fact this whole file, is a big mess. We need to
// get location in libo and use it. Then we need to make this only track source
// locations, and we should emit a source location lazily, only right before
// emitting some other directive or byte count. Currently we flush and emit
// right away which makes no sense, it's backwards.
void emit_source_location(const char* /*nullable*/ filename, int line) {
    //printf("set source %s:%i output_pass %i\n",filename,line, output_pass);
    if (!output_pass) {
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

void emit_symbol_debug(string_t* name) {
    if (!output_pass) {
        return;
    }
    if (!option_debug) {
        return;
    }

    // if the symbol hasn't changed, do nothing
    if (emit_current_symbol != NULL && string_equal(name, emit_current_symbol)) {
        return;
    }

    emit_byte_count();

    if (emit_current_symbol) {
        string_deref(emit_current_symbol);
    }
    emit_current_symbol = string_ref(name);

    fprintf(debug_file, "#symbol %s\n", emit_current_symbol->bytes);
}

void emit_increment_line(int line) {
    emit_source_location(NULL, emit_line + 1);
}
