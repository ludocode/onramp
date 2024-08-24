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

/*
 * This is the Onramp second stage linker. It is written in omC.
 *
 * See the attached README.md for details.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef __onramp__  // TODO fix this
#include <sys/stat.h>
#endif

#include "libo-error.h"
#include "libo-util.h"



/*
 * Definitions
 */

/** A label or symbol definition. */
typedef char definition_t;

//static int total_labels;
definition_t* definition_new(void) {
    //++total_labels;
    return malloc(
            (sizeof(void*) +  // next
            sizeof(void*)) +  // name
            sizeof(int));       // address
}

definition_t** definition_next(definition_t* definition) {
    return (definition_t**)definition;
}

char** definition_name(definition_t* definition) {
    return (char**)(definition + sizeof(void*));
}

int* definition_address(definition_t* definition) {
    return (int*)(definition + (2 * sizeof(void*)));
}

void definition_delete(definition_t* definition) {
    free(*definition_name(definition));
    free(definition);
}



/*
 * Global Variables
 */

static definition_t** globals;
static definition_t** locals;
#define TABLE_SIZE 128
#define TABLE_MASK 0x7F

static int current_char;
static int current_address;
static int file_start_address;

static FILE* output_file;
static FILE* input_file;
static long file_start_pos;
static int pass;
static char file_first_char;
static const char* wrap_header;

static char* buffer;
#define BUFFER_SIZE 128



/*
 * Hashtable functions
 *
 * We use small closed hashtables with instrusive linked lists for collision
 * resolution.
 */

// TODO this is now in libo
static int fnv1a(const char* s) {
    // TODO it is probably undefined behaviour to do this signed, we should
    // make it unsigned when not compiling for Onramp
    int hash;
    hash = -2128831035;
    while (*s != 0) {
        hash = ((hash ^ (*s & 0xFF)) * 16777619);
        s = (s + 1);
    }
    return hash;
}

/**
 * Deletes all definitions in the given table.
 */
static void table_clear(definition_t** table) {
    int i;
    i = 0;
    while (i < TABLE_SIZE) {
        definition_t* definition;
        definition = *(table + i);

        while (definition != NULL) {
            definition_t* next;
            next = *definition_next(definition);
            definition_delete(definition);
            definition = next;
        }

        *(table + i) = NULL;
        i = (i + 1);
    }
}

/**
 * Returns the definition with the given name in the table, or returns NULL if
 * not found.
 */
static definition_t* table_find(definition_t** table, const char* name) {
    definition_t* it;
    it = *(table + (fnv1a(name) & TABLE_MASK));
    while (it != NULL) {
        if (0 == strcmp(name, *definition_name(it))) {
            return it;
        }
        it = *definition_next(it);
    }
    return NULL;
}

/**
 * Inserts a new definition into the given table.
 *
 * The table is scanned for duplicates. If a duplicate is found, it's a fatal
 * error.
 */
static void table_insert(definition_t** table, definition_t* definition) {
    const char* name;
    name = *definition_name(definition);
    //printf("inserting %s\n", name);

    // get a pointer to the hashtable bucket
    definition_t** it;
    it = (table + (fnv1a(name) & TABLE_MASK));

    // scan to the end of the list, checking for duplicates
    while (*it != NULL) {
        if (0 == strcmp(name, *definition_name(*it))) {
            fatal("Duplicate definition");
        }
        it = definition_next(*it);
    }

    // append to the list
    *it = definition;
    *definition_next(definition) = NULL;
}



/*
 * Emit
 */

void emit_byte(char c) {
    if (pass == 2) {
        fputc(c, output_file);
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



/*
 * Parsing
 */

/** Starts a new file, either a real file or a file in a static archive. */
static void start_file(const char* new_filename) {
    set_current_filename(new_filename);
    if (pass != 0) {
        table_clear(locals);
    }
}

static void save_file_state(void) {
    // We store the state of all these things so we can restore them later.
    file_start_pos = ftell(input_file);
/*
fputs("ftell returned ",stderr);
fputd(file_start_pos,stderr);
fputs("\n",stderr);
*/
    file_start_address = current_address;
    file_first_char = current_char;
}

/** Restarts a file for another parsing pass. */
static void restore_file_state(void) {
    /*
fputs("seek ", stdout);
fputs(current_filename, stdout);
fputs(" ", stdout);
fputd(file_start_pos, stdout);
fputc('\n', stdout);
*/
    fseek(input_file, file_start_pos, SEEK_SET);
/*
int t = ftell(input_file);
fputs("fseek to ",stderr);
fputd(file_start_pos,stderr);
fputs(" ftell is now ",stderr);
fputd(t,stderr);
fputs("\n",stderr);
*/
    current_line = 1;
    current_address = file_start_address;
    current_char = file_first_char;
}

static void next_char(void) {
    //printf("last char %x\n",current_char);
    //fputs("reading char...\n",stderr);
    if (1 != fread(&current_char, 1, 1, input_file)) {
        //fputs("EOF\n",stderr);
        if (!feof(input_file)) {
            fatal("Failed to read input file.");
        }
        current_char = EOF;
    }
    //printf("read char %x\n",current_char);
}

static void read_label(void) {
    // The first character can't be a digit
    if ((!isalpha(current_char)) & ((current_char != '_') & (current_char != '$'))) {
        //printf("%c\n", current_char);
        //fputd(current_char,stderr);
        fatal("Expected label or symbol name to start with a letter, underscore or dollar sign.");
    }

    *buffer = current_char;
    int length;
    length = 1;
    next_char();

    // Subsequent chars can be digits
    while (isalnum(current_char) | ((current_char == '_') | (current_char == '$'))) {
        *(buffer + length) = current_char;
        length = (length + 1);
        if (length == BUFFER_SIZE) {
            fatal("Identifier is too long.");
        }
        next_char();
    }

    *(buffer + length) = 0;
}

static bool try_parse_whitespace(void) {
    if (!isspace(current_char)) {
        return false;
    }

    bool was_carriage_return;
    was_carriage_return = (current_char == '\r');
    if (was_carriage_return | (current_char == '\n')) {
        current_line = (current_line + 1);
    }
    next_char();

    // carriage return followed by line feed counts as a single line ending.
    if (was_carriage_return & (current_char == '\n')) {
        next_char();
    }

    return true;
}

static bool try_parse_comment(void) {
    // TODO not doing debug info for now, treat as comments
    if ((current_char != ';') & (current_char != '#')) {
        return false;
    }

    // comment found. consume it
    while (1) {
        next_char();
        if ((current_char == '\r') | (current_char == '\n')) {
            break;
        }
    }

    // note that we don't consume the carriage return or line feed. we let
    // try_parse_whitespace() do it so it handles line endings correctly.

    return true;
}

static int hex_to_int(char c) {
    if ((c >= '0') & (c <= '9')) {
        return c - '0';
    }
    if ((c >= 'a') & (c <= 'f')) {
        return (c - 'a') + 10;
    }
    if ((c >= 'A') & (c <= 'F')) {
        return (c - 'A') + 10;
    }
    fatal("Expected hexadecimal character");
}

static bool try_parse_hex(void) {
    if (!isxdigit(current_char)) {
        return false;
    }

    int value;
    value = (hex_to_int(current_char) << 4);
    next_char();
    value = (value | hex_to_int(current_char));
    next_char();

    current_address = (current_address + 1);
    emit_byte(value);
    return true;
}

static bool try_parse_invoke(void) {
    char type;
    type = current_char;
    if (((type != '&') & (type != '^')) & ((type != '<') & (type != '>'))) {
        return false;
    }

    // increment current address. if ^ it's 4, otherwise it's 2.
    if (type == '^') {
        current_address = (current_address + 4);
    }
    if (type != '^') {
        current_address = (current_address + 2);
    }

    // read the label name into the buffer
    /*
fputs("invoke ",stderr);
fputc(current_char,stderr);
fputs(" ",stderr);
fputd(current_char,stderr);
fputc('\n',stderr);
*/
    next_char();
    read_label();

    // we only check for validity and emit the address in pass 2
    if (pass != 2) {
        return true;
    }

    // find it
    definition_t* definition;
    definition = table_find(globals, buffer);
    if (definition == NULL) {
        definition = table_find(locals, buffer);
    }
    if (definition == NULL) {
        fatal("Undefined label");
    }

    // emit the address
    int address;
    address = *definition_address(definition);
    if (type == '^') {
        emit_int(address);
    }
    if (type == '<') {
        emit_short(address >> 16);
    }
    if (type == '>') {
        emit_short(address);
    }
    if (type == '&') {
        int offset;
        offset = (address - current_address);
        if ((offset < -0x8000) | (offset > 0xFFFF)) {
            fatal("Relative invocation out of bounds.");
        }
        if (offset & 0x3) {
            fatal("Relative invocation is misaligned.");
        }
        offset = (offset >> 2);
        emit_short(offset);
    }
    return true;
}

static bool try_parse_define(void) {
    char type;
    type = current_char;
    if (((type != '=') & (type != '@')) & (type != ':')) {
        return false;
    }
    //printf("define %c addr %i\n",type,current_address);

    // if it's a symbol, pad with zeroes to align to a word boundary
    if (type != ':') {
        while (current_address & 3) {
            //printf("inserting padding byte\n");
            emit_byte(0);
            current_address = (current_address + 1);
        }
    }

    // read the label name into the buffer
    /*
fputs("define ",stderr);
fputc(current_char,stderr);
fputs(" ",stderr);
fputd(current_char,stderr);
*/
    next_char();
    /*
fputs(" ",stderr);
fputd(current_char,stderr);
fputc('\n',stderr);
*/
    read_label();

    // globals are defined in pass 0.
    definition_t** table;
    if (type == '=') {
        if (pass != 0) {
            return true;
        }
        table = globals;
    }

    // locals are defined in pass 1.
    if (type != '=') {
        if (pass != 1) {
            return true;
        }
        table = locals;

        // check that this local isn't already defined as a global
        if (table_find(globals, buffer)) {
            fatal("Duplicate definition");
        }
    }

    // allocate the definition
    char* name;
    definition_t* definition;
    name = strdup(buffer);
    definition = definition_new();
    if ((name == NULL) | (definition == NULL)) {
        fatal("Out of memory.");
    }
    *definition_next(definition) = NULL;
    *definition_name(definition) = name;
    *definition_address(definition) = current_address;

    // insert it (this will check for duplicates in the table)
    table_insert(table, definition);
    return true;
}

/** Parses a metadata line (file delimiter) in a static archive. */
static bool try_parse_archive(void) {
    if (current_char != '%') {
        return false;
    }
    //printf("found archive line, pass %i\n", pass);

    // If we're in pass 1, we jump back to the top and start pass 2.
    if (pass == 1) {
        //printf("restoring file state, moving to pass 2\n");
        pass = 2;
        restore_file_state();
        // We haven't parsed the line ending of the archive metadata yet so we
        // start at line 0.
        current_line = 0;
        return true;
    }

    // Read the filename
    next_char();
    int length;
    length = 0;
    while (((current_char != '\n') & (current_char != '\r')) & (current_char != EOF)) {
        *(buffer + length) = current_char;
        length = (length + 1);
        if (length == BUFFER_SIZE) {
            fatal("Filename of file in archive is too long.");
        }
        next_char();
    }
    *(buffer + length) = 0;
    //printf("archive line file: %s\n",buffer);

    // If we're in pass 2, we'll start pass 1 for the next file
    if (pass == 2) {
        //printf("saving file state, moving to pass 1\n");
        pass = 1;
        save_file_state();
    }

    // We haven't parsed the line ending of the archive metadata yet so we
    // start at line 0.
    current_line = 0;
    start_file(buffer);
    return true;
}

/** Performs one step of parsing. */
static void parse(void) {
    //printf("char %c %x\n",current_char,current_char);
    if (try_parse_whitespace()) {
        return;
    }
    if (try_parse_hex()) {
        return;
    }
    if (try_parse_comment()) {
        return;
    }
    if (try_parse_invoke()) {
        return;
    }
    if (try_parse_define()) {
        return;
    }
    if (try_parse_archive()) {
        return;
    }
    fatal("Invalid character.");
}

/** Performs a full pass of parsing on the given input file. */
static void perform_pass_input(const char* input_filename) {
    start_file(input_filename);
    current_line = 0;

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fatal("Failed to open input file.");
    }

    current_char = 0;
    next_char();
    save_file_state();
    current_line = 1;

    while (1) {
        if (current_char == EOF) {
            if (pass == 1) {
                restore_file_state();
                pass = 2;
                continue;
            }
            if (pass == 2) {
                // Pass two of this file or archive is done. Switch back to
                // pass one for the next file.
                pass = 1;
            }
            break;
        }
        parse();
    }

    fclose(input_file);
}

static void open_output(const char* output_filename) {
    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fatal("Failed to open output file.");
    }

    // TODO this can be removed now that we bootstrap ld/2, no reason to
    // support this in an earlier stage
    if (wrap_header != NULL) {
        chmod(output_filename, 493); // 493 == 0755, cci/0 doesn't support octal

        FILE* header = fopen(wrap_header, "r");
        if (header == NULL) {
            fatal("Failed to open wrap header file.");
        }

        size_t buffer_size = 128;
        char* buffer = calloc(1, buffer_size);
        if (buffer == NULL) {
            fatal("Out of memory.");
        }
        // TODO check for errors, loop for buffering, etc. This code is
        // temporary anyway, we won't be using it once we can build ld/2.
        fread(buffer, 1, buffer_size, header);
        fclose(header);
        fwrite(buffer, 1, buffer_size, output_file);
        free(buffer);
    }
}

static void perform_pass(const char** argv) {
    current_address = 0;

    while (*argv != 0) {
        if (0 == strcmp(*argv, "-wrap-header")) {
            argv = (argv + 1);
            if (*argv == 0) {
                fatal("-wrap-header must be followed by a filename.");
            }
            if (wrap_header == NULL) {
                if (output_file != NULL) {
                    fatal("-wrap-header must come before -o.");
                }
                wrap_header = *argv;
            }
            argv = (argv + 1);
            continue;
        }

        if (0 == strcmp(*argv, "-o")) {
            argv = (argv + 1);
            if (*argv == 0) {
                fatal("-o must be followed by an output file.");
            }
            if (pass == 0) {
                if (output_file != NULL) {
                    fatal("Only one -o output file can be specified.");
                }
                open_output(*argv);
            }
            argv = (argv + 1);
            continue;
        }

        perform_pass_input(*argv);
        argv = (argv + 1);
    }
}

int main(int argc, const char** argv) {
    globals = calloc(TABLE_SIZE, sizeof(void*));
    locals = calloc(TABLE_SIZE, sizeof(void*));
    buffer = malloc(BUFFER_SIZE);

    // skip program name
    argv = (argv + 1);

    pass = 0;
    perform_pass(argv);

    if (output_file == NULL) {
        fatal("No -o output file specified.");
    }

    pass = 1;
    perform_pass(argv);

    set_current_filename(NULL);
    free(buffer);
    table_clear(locals);
    free(locals);
    table_clear(globals);
    free(globals);
    //printf("total %i\n",total_labels);
    return EXIT_SUCCESS;
}
