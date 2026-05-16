/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#include "symbol.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "emit.h"
#include "libo-table.h"
#include "libo-vector.h"

/*
 * The assembler accumulates all file data except for labels (hex bytes, debug
 * symbols and whitespace) into the symbol buffer, tracking the byte position
 * and hex offset as it goes.
 *
 * Once the full symbol is parsed, we walk through the labels emitting all data
 * in between. If a label can be resolved, we emit the resolved value;
 * otherwise we emit the label as-is and let the linker figure it out.
 *
 * The word "offset" is used to refer to a number of hexadecimal bytes (which
 * will result in real bytes in the bytecode), while the word "position" or
 * "count" is used for the number of bytes including whitespace and debug info.
 * Each hex byte therefore increments the offset by one and the count by two.
 */

static uint8_t* symbol_buffer;
static size_t symbol_count; // number of bytes in the buffer
static size_t symbol_capacity;
static size_t symbol_offset; // current hex offset

static table_t definitions; // contains label_t* (unowned)
static vector_t labels;     // contains label_t* (owned)

typedef struct {
    table_entry_t entry;
    char* name;
    label_type_t type;
    size_t offset;   // hex offset within the symbol
    size_t position; // position in the buffer
    int flags;
} label_t;

static void label_delete(label_t* label) {
    free(label->name);
    free(label);
}

bool symbol_is_aligned(void) {
    return (symbol_offset & 3) == 0;
}

void symbol_setup(void) {
    table_init(&definitions);
    table_reserve_bits(&definitions, 8); // 256 buckets, 1 kB
    vector_init(&labels);
    vector_reserve(&labels, 256); // 1 kB
}

void symbol_clear(void) {

    // clear buffer
    symbol_count = 0;
    symbol_offset = 0;

    // clear definitions
    table_remove_all(&definitions);

    // clear labels
    for (size_t i = vector_count(&labels); i-- > 0;) {
        label_delete(vector_at(&labels, i));
    }
    vector_remove_all(&labels);
}

void symbol_teardown(void) {
    symbol_clear();
    vector_destroy(&labels);
    table_destroy(&definitions);
    free(symbol_buffer);
}

/**
 * Creates space for the given additional number of bytes, returning a pointer
 * to them.
 */
static uint8_t* symbol_add_space(size_t count) {
    size_t old_count = symbol_count;
    size_t new_count = old_count + count;
    if (new_count < old_count) {
        fatal("Overflow.");
    }

    // grow if necessary
    if (new_count > symbol_capacity) {
        size_t new_capacity = (symbol_capacity == 0) ? 8 : symbol_capacity * 2;
        while (new_capacity < new_count) {
            if (new_capacity < symbol_capacity) {
                fatal("Overflow.");
            }
            new_capacity *= 2;
        }
        symbol_buffer = realloc(symbol_buffer, new_capacity);
        if (symbol_buffer == NULL) {
            fatal("Out of memory.");
        }
        symbol_capacity = new_capacity;
    }

    symbol_count = new_count;
    return symbol_buffer + old_count;
}

void symbol_add_byte(uint8_t byte) {
    *symbol_add_space(1) = byte;
}

void symbol_add_bytes(const uint8_t* bytes, size_t count) {
    memcpy(symbol_add_space(count), bytes, count);
}

void symbol_add_hex_byte(uint8_t byte) {
    symbol_add_byte(int_to_hex(byte >> 4));
    symbol_add_byte(int_to_hex(byte & 0xF));
    ++symbol_offset;
}

void symbol_add_hex_bytes(const uint8_t* bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        symbol_add_hex_byte(bytes[i]);
    }
}

void symbol_add_label(const char* name, label_type_t type, int flags) {
    char* name_copy = strdup(name);
    label_t* label = malloc(sizeof(label_t));
    if (label == NULL || name_copy == NULL) {
        fatal("Out of memory.");
    }

    switch (type) {
        case label_type_invocation_absolute:
            symbol_offset += 4;
            break;
        case label_type_invocation_high:
        case label_type_invocation_low:
        case label_type_invocation_relative:
            symbol_offset += 2;
            break;
        default:
            break;
    }

    label->name = name_copy;
    label->position = symbol_count;
    label->offset = symbol_offset;
    label->type = type;
    label->flags = flags;

    if (type == label_type_invocation_relative && (symbol_offset & 3) != 0) {
        fatal("Misaligned relative invocation.");
    }

    vector_append(&labels, label);

    if (type == label_type_definition_label) {
        table_put(&definitions, &label->entry, fnv1a_cstr(name));
    }
}

/**
 * Returns the definition for the given label or NULL.
 */
label_t* symbol_find_definition(const char* name) {
    for (table_entry_t* entry = table_bucket(&definitions, fnv1a_cstr(name));
            entry; entry = table_entry_next(entry))
    {
        label_t* definition = (label_t*)entry;
        if (0 == strcmp(name, definition->name)) {
            return definition;
        }
    }
    return NULL;
}

void symbol_emit(void) {
    size_t position = 0;
    size_t label_count = vector_count(&labels);

    for (size_t i = 0; i < label_count; ++i) {
        label_t* label = vector_at(&labels, i);

        // emit all bytes since the last label
        size_t step = label->position - position;
        emit_bytes(symbol_buffer + position, step);
        position = label->position;

        // If the label is a relative invocation we can resolve it now.
        if (label->type == label_type_invocation_relative) {
            label_t* definition = symbol_find_definition(label->name);

            // TODO we should track the original location of these labels
            // so that these error messages end up on the correct lines.
            // This will be easier to fix once location_t is in libo.

            if (!definition) {
                fatal("No definition found for relative invocation.");
            }

            // The invocation alignment was checked when it was parsed.
            // Definitions only need to be aligned if used as the
            // destination of a relative invocation so we check now.
            if ((definition->offset & 3) != 0) {
                fatal("Misaligned relative invocation destination.");
            }

            ssize_t jump_offset = (ssize_t)(definition->offset - label->offset) >> 2;
            if (jump_offset < -0x8000 || jump_offset > 0x7FFF) {
                fatal("Relative jump is too large! Please file a bug against Onramp."
                        " In the meantime you can try shrinking this function.");
            }
            emit_hex_byte(jump_offset & 0xFF);
            emit_hex_byte(((unsigned)jump_offset >> 8) & 0xFF);
            continue;
        }

        // Label definitions can now be dropped.
        if (label->type == label_type_definition_label) {
            continue;
        }

        // Otherwise it's an invocation to a symbol. We emit it for the linker.
        emit_label(label->name, label->type, label->flags, -1, -1);
    }

    // emit all remaining bytes
    emit_bytes(symbol_buffer + position, symbol_count - position);

    symbol_clear();
}
