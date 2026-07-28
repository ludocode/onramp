/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include <stdio.h>
#include <ctype.h>

#include "block.h"
#include "common.h"
#include "function.h"
#include "libo-error.h"
#include "libo-util.h"
#include "node.h"
#include "options.h"
#include "symbol.h"
#include "token.h"

static FILE* output_file;
static token_t* current_location;

void emit_setup(const char* output_filename) {
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        fatal("ERROR: Failed to open output file.");
    }
    emit_cstr("#line manual\n");
    emit_global_divider();
}

void emit_teardown(void) {
    if (current_location)
        token_deref(current_location);
    fclose(output_file);
}

void emit_char(char c) {
    fputc(c, output_file);
}

void emit_cstr(const char* cstr) {
    fputs(cstr, output_file);
}

void emit_string(const string_t* string) {
    fwrite(string->bytes, 1, string->length, output_file);
}

void emit_number(int number) {
    char buffer[12];
    emit_cstr(itoa_d(number, buffer));
}

void emit_hex_number(int number) {
    fprintf(output_file, "%X", number);
}

void emit_newline(void) {
    emit_char('\n');
}

void emit_global_divider(void) {
    emit_cstr("\n\n\n");
}

void emit_arg_number(int number) {
    emit_char(' ');
    emit_number(number);
}

void emit_arg_invocation_prefix(char sigil, const char* prefix, int number) {
    emit_char(' ');
    emit_char(sigil);
    emit_cstr(prefix);
    if (number != -1) {
        emit_hex_number(number);
    }
}

void emit_arg_invocation(char sigil, const char* label) {
    emit_char(' ');
    emit_char(sigil);
    emit_cstr(label);
}

static void emit_label_def(const char* prefix, int number, string_t* /*nullable*/ user_label) {
    emit_char(':');
    emit_cstr(prefix);
    emit_hex_number(number);
    if (user_label) {
        emit_cstr(" ; ");
        emit_string(user_label);
    }
    emit_newline();
}

static char int_to_hex(unsigned value) {
    if (value <= 9) {
        return '0' + value;
    }
    if (value <= 15) {
        return 'A' + (value - 10);
    }
    fatal("Internal error: invalid hex value");
}

static void emit_hex_char(unsigned nibble) {
    emit_char(int_to_hex(nibble));
}

static void emit_hex_byte(uint8_t byte) {
    emit_hex_char(byte >> 4);
    emit_hex_char(byte & 0xF);
}

static bool is_string_char_valid_assembly(char c) {

    // these characters are invalid in a string in Onramp assembly
    if (c == '\\' || c == '"') {
        return false;
    }

    // otherwise it must be a printable character
    return isprint(c);
}

void emit_quoted_byte(char byte) {
    emit_char('\'');
    emit_hex_byte(byte);
}

void emit_string_literal(const string_t* str) {
    bool open = false;

    const char* p = str->bytes;
    const char* end = p + str->length;
    while (p != end) {
        char c = *p++;
        bool valid = is_string_char_valid_assembly(c);
        if (valid != open) {
            emit_char('"');
            open = !open;
        }
        if (valid) {
            emit_char(c);
        } else {
            emit_quoted_byte(c);
        }
    }

    if (open) {
        emit_char('"');
    }
}

/**
 * Emits the given block.
 */
static void emit_block(function_t* function, block_t* block) {
    assert(!block->emitted);
    block->emitted = true;

    emit_label_def(JUMP_LABEL_PREFIX, block->label, block->user_label);

    size_t count = block->instructions_count;

    #ifdef DEBUG
    // get the last instruction
    if (count == 0) {
        fatal("Internal error: a basic block cannot be empty.");
    }
    instruction_t* last = block_at(block, count - 1);

    // make sure the block ends properly
    if (last->opcode != JMP && last->opcode != RET && last->opcode != BR) {
        fatal("Internal error: a basic block must end in BR, JMP or RET.");
    }
    #endif

    for (size_t i = 0; i < count; ++i) {
        instruction_emit(block_at(block, i));
    }
}

void emit_function(function_t* function) {
    symbol_t* symbol = function->symbol;

    // output flags
    emit_char(symbol->linkage == symbol_linkage_internal ? '@' : '=');
    if (symbol->is_weak) {
        emit_char('?');
    }
    if (symbol->is_constructor) {
        emit_char('{');
        if (symbol->constructor_priority >= 0)
            emit_number(symbol->constructor_priority);
    }
    if (symbol->is_destructor) {
        emit_char('{');
        if (symbol->destructor_priority >= 0)
            emit_number(symbol->destructor_priority);
    }

    emit_string(function->asm_name);
    emit_newline();

    // emit return value parameter (if indirect)
    if (function->return_temporary != TEMPORARY_INVALID) {
        emit_cstr("  param ");
        emit_string(temporary_name(function->return_temporary));
        emit_char('\n');
    }

    // emit parameters
    node_t* root = function->root;
    for (node_t* param = root->first_child;
            param != root->last_child;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);
        emit_cstr("  param ");
        symbol_t* symbol = param->symbol;
        if (symbol) {
            int temporary;
            if (symbol->indirect_parameter_temporary != TEMPORARY_INVALID) {
                temporary = symbol->indirect_parameter_temporary;
            } else {
                temporary = symbol->temporary;
            }
            emit_string(temporary_name(temporary));
        } else {
            emit_char('%'); // sentinel, param ignored
        }
        emit_char('\n');
    }
    if (function->variadic_temporary != -1) {
        emit_cstr("  varargs ");
        emit_string(temporary_name(function->variadic_temporary));
        emit_char('\n');
    }

    // emit variable declarations
    function_emit_variables(function);

    size_t count = vector_count(&function->blocks);
    for (size_t i = 0; i < count; ++i) {
        block_t* block = vector_at(&function->blocks, i);
        if (!block->emitted) {
            emit_block(function, block);
        }
    }
}

static void emit_source_location_full(token_t* token) {
    fprintf(output_file, "#line %i ", token->line);
    emit_string_literal(token->filename);
    emit_char('\n');
}

void emit_source_location(token_t* /*nullable*/ token) {
    if (!token)
        return;

    if (current_location == NULL) {
        emit_source_location_full(token);
        current_location = token_ref(token);
        return;
    }

    if (!string_equal(token->filename, current_location->filename)) {
        emit_source_location_full(token);
    } else if (token->line == current_location->line) {
        // nothing
    } else if (token->line == current_location->line + 1) {
        emit_cstr("#\n");
    } else {
        fprintf(output_file, "#line %i\n", token->line);
    }

    token_ref(token);
    token_deref(current_location);
    current_location = token;
}
