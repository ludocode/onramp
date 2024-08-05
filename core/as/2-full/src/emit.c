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

#include "parse.h"

FILE* output_file;
size_t output_alignment;

static char label_type_to_char(label_type_t type) {
    switch (type) {
        case label_type_invocation_absolute: return '^';
        case label_type_invocation_high: return '<';
        case label_type_invocation_low: return '>';
        case label_type_invocation_relative: return '&';
        case label_type_definition_label: return ':';
        case label_type_definition_symbol: return '=';
        case label_type_definition_static: return '@';
    }
    fatal("Internal error: invalid label type");
}

void emit_label(const char* name, label_type_t type, int flags) {

    // track alignment
    switch (type) {
        case label_type_invocation_high: // fallthrouh
        case label_type_invocation_low: // fallthrouh
        case label_type_invocation_relative:
            output_alignment = (output_alignment + 2) & 3;
            break;
        case label_type_definition_symbol: // fallthrough
        case label_type_definition_static:
            output_alignment = 0;
            break;
        default:
            break;
    }

    // emit label
    emit_char(label_type_to_char(type));
    if (flags & LABEL_FLAG_WEAK)
        emit_char('?');
    if (flags & LABEL_FLAG_ZERO)
        emit_char('+');
    if (flags & LABEL_FLAG_CONSTRUCTOR)
        emit_char('{');
    if (flags & LABEL_FLAG_DESTRUCTOR)
        emit_char('}');
    fputs(name, output_file);

    // Always follow a label with a space to ensure we aren't concatenating
    // subsequent hex chars to it
    emit_char(' ');
}

void emit_imw_absolute(uint8_t reg) {
    emit_hex_byte(0x7C); // ims
    emit_hex_byte(reg);
    emit_label(identifier, label_type_invocation_high, label_flags);
    emit_hex_byte(0x7C); // ims
    emit_hex_byte(reg);
    emit_label(identifier, label_type_invocation_low, label_flags);
}

static char int_to_hex(unsigned value) {
    if (value <= 9)
        return '0' + value;
    if (value <= 15)
        return 'A' + value - 10;
    fatal("Internal error: invalid hex value");
}

void emit_char(char c) {
    fputc(c, output_file);
}

void emit_hex_byte(uint8_t byte) {
    emit_char(int_to_hex(byte >> 4));
    emit_char(int_to_hex(byte & 0xF));
    output_alignment = (output_alignment + 1) & 3;
}

void emit_hex_bytes(const uint8_t* bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        emit_hex_byte(bytes[i]);
    }
}

void emit_line_directive(int line, const char* /*nullable*/ filename) {
    fputs("#line ", output_file);
    fprintf(output_file, "%i", line);
    if (filename) {
        fputc(' ', output_file);
        fputc('"', output_file);
        fputs(filename, output_file);
        fputc('"', output_file);
    }
    fputc('\n', output_file);
}
