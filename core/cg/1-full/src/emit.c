/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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

#include "argument.h"
#include "block.h"
#include "common.h"
#include "instruction.h"
#include "libo-error.h"
#include "libo-string.h"
#include "location.h"
#include "symbol.h"

static void emit_location_force(void);

void emit_setup(void) {
    emit_location_force();
}

void emit_teardown(void) {
}

static void emit_char(char c) {
    fputc(c, output_file);
}

static void emit_cstr(const char* cstr) {
    fputs(cstr, output_file);
}

static void emit_string(const string_t* string) {
    fwrite(string->bytes, 1, string->length, output_file);
}

static void emit_uint(uint32_t value) {
    fprintf(output_file, "%u", value);
}

static void emit_location_force(void) {
    emit_cstr("#line ");
    emit_uint(current_line);
    emit_char(' ');
    emit_string(current_filename_string);
    emit_char('\n');
}

static void emit_location(location_t* location) {
    assert(location);
fprintf(stderr, "%s() %s:%i %p %p\n", __func__, __FILE__, __LINE__, (void*)location, (void*)current_filename_string);

    if (!string_equal(location->filename, current_filename_string)) {
        set_current_filename_string(location->filename);
        current_line = location->line;
        // filename has changed. emit full debug info.
        emit_location_force();
        return;
    }

    // TODO remove all these unsigned casts once we make current_line unsigned

    if (location->line == (unsigned)current_line) {
        return;
    }

    if (current_line != 0 && location->line != 0
            && location->line > (unsigned)current_line
            && location->line < (unsigned)current_line + 5)
    {
        for (; location->line < (unsigned)current_line; ++current_line) {
            emit_char('#');
            emit_char('\n');
        }
        return;
    }

    current_line = location->line;
    emit_cstr("#line ");
    emit_uint(current_line);
    emit_char('\n');
    return;
}

static void emit_register(uint32_t num) {
    switch (num) {
        case 0: emit_cstr("r0"); break;
        case 1: emit_cstr("r1"); break;
        case 2: emit_cstr("r2"); break;
        case 3: emit_cstr("r3"); break;
        case 4: emit_cstr("r4"); break;
        case 5: emit_cstr("r5"); break;
        case 6: emit_cstr("r6"); break;
        case 7: emit_cstr("r7"); break;
        case 8: emit_cstr("r8"); break;
        case 9: emit_cstr("r9"); break;
        case 10: emit_cstr("ra"); break;
        case 11: emit_cstr("rb"); break;
        case 12: emit_cstr("rsp"); break;
        case 13: emit_cstr("rfp"); break;
        case 14: emit_cstr("rpp"); break;
        case 15: emit_cstr("rip"); break;
        default:
            fatal("Internal error: invalid register");
            break;
    }
}

static void emit_argument(argument_t* argument) {
    emit_char(' ');
    switch (argument->type) {
        case argument_type_sentinel:
            // This only exists for debugging purposes.
            emit_char('%');
            break;
        case argument_type_temporary:
            // This only exists for debugging purposes.
            emit_string(argument->string);
            break;
        case argument_type_register:
            emit_register(argument->number);
            break;
        case argument_type_number:
            emit_uint(argument->number);
            break;
        case argument_type_absolute:
            emit_char('^');
            emit_string(argument->string);
            break;
        case argument_type_relative:
            emit_char('&');
            emit_string(argument->string);
            break;
        default:
            fatal("Internal error: invalid argument type");
            break;
    }
}

static void emit_instruction(instruction_t* instruction) {
    emit_location(instruction->location);
    emit_cstr(opcode_to_string(instruction->opcode));
    for (size_t i = 0; i < vector_count(&instruction->arguments); ++i) {
        emit_argument(vector_at(&instruction->arguments, i));
    }
    emit_char('\n');
}

void emit_symbol(symbol_t* symbol) {

    // emit the symbol name
    emit_location(symbol->location);
    emit_char('=');
    emit_cstr(symbol->name);
    emit_char('\n');

    // emit the blocks
    // TODO don't emit any unreachable blocks

    for (size_t i = 0; i < vector_count(symbol->blocks); ++i) {
        block_t* block = vector_at(symbol->blocks, i);
        emit_location(block->location);
        emit_char(':');
        emit_cstr(block->name);
        emit_char('\n');

        for (size_t i = 0; i < vector_count(block->instructions); ++i) {
            emit_instruction(vector_at(block->instructions, i));
        }
    }
}
