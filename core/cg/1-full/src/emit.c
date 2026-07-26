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
#include "temporary.h"
#include "variable.h"

static string_t* last_filename;
static unsigned last_line;

static void emit_location_force(location_t* location);

static void emit_set_last_filename(string_t* /*nullable*/ filename) {
    if (filename) {
        string_ref(filename);
    }
    if (last_filename) {
        string_deref(last_filename);
    }
    last_filename = filename;
}

void emit_setup(void) {
    location_t* location = location_new_current();
    emit_location_force(location);
    location_delete(location);
}

void emit_teardown(void) {
    emit_set_last_filename(NULL);
}

void emit_char(char c) {
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

static void emit_int(uint32_t value) {
    fprintf(output_file, "%i", (int32_t)value);
}

static void emit_location_force(location_t* location) {
    emit_cstr("#line ");
    emit_uint(location->line);
    emit_char(' ');
    emit_char('"');
    emit_string(location->filename); // TODO escape properly
    emit_char('"');
    emit_char('\n');
    emit_set_last_filename(location->filename);
    last_line = location->line;
}

static void emit_location(location_t* location) {
    assert(location);

    if (!last_filename || !string_equal(location->filename, last_filename)) {
        // filename has changed. emit full debug info.
        emit_location_force(location);
        return;
    }

    if (location->line == last_line) {
        return;
    }

    if (last_line != 0 && location->line != 0
            && location->line > last_line
            && location->line < last_line + 5)
    {
        for (; location->line > last_line; ++last_line) {
            emit_char('#');
            emit_char('\n');
        }
        return;
    }

    last_line = location->line;
    emit_cstr("#line ");
    emit_uint(last_line);
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
            emit_string(argument_temporary(argument)->name);
            break;
        case argument_type_register:
            emit_register(argument->number);
            break;
        case argument_type_number:
            //printf("arg number %u\n", argument->number);
            emit_int(argument->number);
            break;
        case argument_type_absolute:
            emit_char('^');
            emit_string(argument->string);
            break;
        case argument_type_relative:
            emit_char('&');
            emit_string(argument->string);
            break;
        case argument_type_variable:
            emit_char('$');
            emit_string(argument->variable->name);
            break;
        default:
            fatal("Internal error: invalid argument type");
            break;
    }
}

static void emit_instruction(instruction_t* instruction) {
    if (instruction->opcode == opcode_nop) {
        #ifdef DEBUG
        emit_cstr(" ; nop\n");
        #endif
        return;
    }

    emit_location(instruction->location);
    emit_char(' ');
    emit_cstr(opcode_to_string(instruction->opcode));
    for (size_t i = 0; i < vector_count(instruction->arguments); ++i) {
        emit_argument(vector_at(instruction->arguments, i));
    }
    emit_char('\n');
}

static void emit_block_append(vector_t* blocks, block_t* child, int visited) {
    if (child->visited != visited) {
        child->visited = visited;
        vector_append(blocks, child);
    }
}

/**
 * Emits the given block, appending any unvisited reachable blocks to the given
 * array.
 *
 * (The register allocator runs only on reachable blocks. Unreachable blocks
 * haven't been transformed to assembly so we can't emit them.)
 */
static void emit_block(symbol_t* symbol, block_t* block, vector_t* blocks, int visited) {

    emit_char('\n');
    emit_location(block->location);
    emit_char(':');
    emit_string(block->name);
    emit_char('\n');

    // emit all instructions
    for (size_t i = 0; i < vector_count(block->instructions); ++i) {
        instruction_t* instruction = vector_at(block->instructions, i);
        emit_instruction(instruction);

        // if this is a jump, add the destination block to the list
        switch (instruction->opcode) {
            case opcode_jmp:
                emit_block_append(blocks,
                        block_find(argument_label(instruction_argument(instruction, 0))),
                        visited);
                break;
            case opcode_jz:
                emit_block_append(blocks,
                        block_find(argument_label(instruction_argument(instruction, 1))),
                        visited);
                break;
            default:
                break;
        }
    }
}

void emit_symbol_name(symbol_t* symbol) {
    emit_location(symbol->location);
    emit_char(symbol->is_static ? '@' : '=');

    if (symbol->constructor_priority != PRIORITY_INVALID) {
        emit_char('{');
        fprintf(output_file, "%zu", symbol->constructor_priority); // must be decimal
    }

    if (symbol->destructor_priority != PRIORITY_INVALID) {
        emit_char('}');
        fprintf(output_file, "%zu", symbol->constructor_priority); // must be decimal
    }

    emit_cstr(symbol->name);
    emit_char('\n');
}

void emit_symbol(symbol_t* symbol) {
    emit_symbol_name(symbol);

    // emit reachable blocks. the vector grows as we iterate.
    vector_t* blocks = vector_new();
    block_t* first = vector_first(symbol->blocks);
    int visited = pass_id++;
    first->visited = visited;
    vector_append(blocks, first);
    for (size_t i = 0; i < vector_count(blocks); ++i) {
        emit_block(symbol, vector_at(blocks, i), blocks, visited);
    }
    vector_delete(blocks);

    emit_char('\n');
    emit_char('\n');
    emit_char('\n');
}
