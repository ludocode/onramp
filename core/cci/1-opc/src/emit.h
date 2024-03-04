#ifndef EMIT_H_INCLUDED
#define EMIT_H_INCLUDED

#include <stdint.h>
#include <stdio.h>

void emit_init(const char* output_filename);
void emit_destroy(void);

void emit_char(char c);
void emit_string(const char* c);

/**
 * Emits a term followed by a space.
 *
 * A "term" is one part of an instruction. It can be basically any string.
 * Usually this is used for instruction opcodes, register names, and simple
 * hardcoded arguments like 0 and 1.
 */
void emit_term(const char* term);

void emit_register(int index);
void emit_label(char type, const char* label_name);
void emit_prefixed_label(char type, const char* prefix, const char* label_name);
void emit_computed_label(char type, const char* prefix, int label);
void emit_int(int32_t value);
void emit_string_literal(const char* str);
void emit_character_literal(char c);
void emit_quoted_byte(uint8_t byte);
void emit_newline(void);
void emit_zero(void);
void emit_global_divider(void);

/**
 * Emits a directive that indicates that a newline was encountered in the
 * source file.
 */
void emit_line_increment_directive(void);

void emit_line_directive(void);

#endif
