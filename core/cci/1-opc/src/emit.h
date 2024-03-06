#ifndef EMIT_H_INCLUDED
#define EMIT_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void emit_init(const char* output_filename);
void emit_destroy(void);

/**
 * Toggles output on or off.
 *
 * When off, nothing is emitted except for line directives; all other emit
 * functions are ignored. This is used to implement sizeof().
 *
 * Output is on by default.
 */
void emit_set_enabled(bool enabled);

bool emit_is_enabled(void);

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
void emit_int(int value);
void emit_string_literal(const char* str);
void emit_character_literal(char c);
void emit_quoted_byte(char byte);
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
