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
 * This is the omC (Onramp Subset of C) preprocessor.
 *
 * It is written in omC, except it depends only on the directive-stripping
 * preprocessor (cpp/0-strip), so it can be bootstrapped without needing a
 * "real" preprocessor.
 *
 * It can also be preprocessed with a real preprocessor, and (in combination
 * with any cci stage) it is self-hosting, though we don't actually bother to
 * build it with itself during the bootstrap process.
 *
 * Since our linker doesn't yet support file scope, we have to implement this
 * in a single C source file (otherwise generated label names would collide.)
 */



/*
 * When bootstrapping, we'll be preprocessing with cpp/0-strip which just
 * strips preprocessor directives. We therefore have to declare all our libc
 * features ourselves.
 *
 * We use the below #include and #ifdef so that this file can still be compiled
 * with an ordinary C compiler for any platform. This makes it easy to test and
 * debug.
 */

/* These #include directives are stripped by cpp/0-strip. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "libo-error.h"
#include "libo-util.h"

/* This macro is never actually defined, but because cpp/0-strip will strip the
 * directives, it's equivalent to the condition being true. With any proper C
 * preprocessor (including this) the condition will be false and the headers
 * above will be included instead. */
#ifdef __onramp_cpp_strip__

    /* <ctype.h> */
    int isalnum(int);
    int isalpha(int);
    int isdigit(int);

    /* <stddef.h> */
    typedef int size_t; /* note: size_t is signed int for now because omC doesn't support unsigned. */

    /* <stdint.h> */
    typedef int intptr_t;

    /* <stdbool.h> */
    typedef int bool;
    int true; // initialized at the top of main()
    int false;

    /* <stdio.h> */
    typedef void FILE;
    FILE* fopen(const char* name, const char* mode);
    int fclose(FILE* file);
    size_t fread(void* buffer, size_t size, size_t count, FILE* file);
    size_t fwrite(const void* buffer, size_t size, size_t count, FILE* file);
    int feof(FILE* file);
    int fputc(int c, FILE* file);
    int fputs(const char* s, FILE* file);
    int puts(const char* s);
    int putchar(int c);
    extern FILE* stdin;
    extern FILE* stdout;
    extern FILE* stderr;

    /* <stdlib.h> */
    void* malloc(size_t size);
    void free(void* p);
    char* strdup(const char* str);
    char* strndup(const char* str, size_t length);
    void _Exit(int status);

    /* <string.h> */
    size_t strlen(const char* s);
    char* strcpy(char* to, const char* from);
    void* memmove(void* dest, const void* src, size_t count);
    int strcmp(const char* left, const char* right);
    void* memcpy(void* dest, const void* src, size_t count);
    int memcmp(const void* a, const void* b, size_t count);
    char* strchr(const char* s, int c);

    /* "libo-error.h" */
    extern char* current_filename;
    extern int current_line;
    void fatal(const char* message);

    /* "libo-util.h" */
    void fputd(int number, FILE* file);

#endif



/*
 * State
 */

/* The current input file from which we're reading characters. When an #include
 * is parsed, this changes and the previous file is stored on the call stack. */
static FILE* current_file;

/* The current character being handled by the preprocessor, after the scanner
 * has stripped comments and escaped newlines. */
static char current_char;

/* The next character after current_char, which has not yet been scanned. */
static char next_scan_char;

const char* input_filename;
const char* output_filename;
const char* force_include_filename;
static FILE* output_file;

/* Depth of true conditionals */
static int depth;

// options
static bool opt_nostddef;
static bool opt_dump_macros;



/*
 * Forward declarations
 */

static void preprocess(const char* new_filename, FILE* new_file);



/*
 * File and error handling helpers
 */

static void write_number(FILE* file, int number) {
    fputd(number, file);
}

static void emit_char(char c) {
    fputc(c, output_file);
}

static void emit_line_directive(void) {
    fputs("#line ", output_file);
    write_number(output_file, current_line);
    emit_char(' ');
    emit_char('"');
    fputs(current_filename, output_file);
    emit_char('"');
    emit_char('\n');
}



/*
 * Scanner
 *
 * The preprocessor doesn't have a tokenizer exactly, but it does have a
 * scanner which does these tasks:
 *
 * - Comments (C and C++ style) are replaced with a single space.
 * - Escaped newlines are eliminated.
 * - Various line ending formats are converted to line feeds.
 * - A newline is appended to the end of the file if one is missing.
 *
 * The scanner otherwise outputs single characters into `current_char`.
 *
 * This is equivalent to the first two phases of translation.
 */

/**
 * Reads a single character from the current file.
 *
 * Returns 0 on end of file. (The input file is not allowed to contain null
 * bytes.)
 */
static char read_char(void) {
    char c;
    if (1 != fread(&c, 1, 1, current_file)) {
        if (!feof(current_file)) {
            fatal("Failed to read input file.");
        }
        return 0;
    }
    if (c == 0) {
        fatal("Input file cannot contain a null byte.");
    }
    return c;
}

/**
 * Consumes current_char, scans for a new character (stripping comments and so
 * on), and places the newly scanned character in current_char.
 *
 * This is the implementation of the character scanner.
 *
 * When the end of the file is reached, a null byte is placed in current_char.
 * (The input file is not allowed to contain null bytes.)
 */
static void next_char(void) {

    // Whenever we consume a newline, bump the line number.
    if (current_char == '\n') {
        emit_char('\n');
        current_line = (current_line + 1);
    }

    current_char = next_scan_char;
    /*
    if (current_char == 0) {
        return;
    }
    */
    next_scan_char = read_char();

    while (1) {

        /* Check for an escaped newline */
        if ((current_char == '\\') & ((next_scan_char == '\n') | (next_scan_char == '\r'))) {
            current_char = read_char();
            if ((next_scan_char == '\r') & (current_char == '\n')) {
                /* Backslash was followed by both carriage return and line
                 * feed; also consume the line feed */
                current_char = read_char();
            }
            if (current_char == 0) {
                /* The file ended in an escaped newline. Synthesize a newline. */
                current_char = '\n';
                next_scan_char = 0;
                break;
            }
            next_scan_char = read_char();
            continue;
        }

        /* Check for a carriage return */
        if (current_char == '\r') {
            if (next_scan_char == '\n') {
                /* CR+LF is replaced by LF */
                current_char = '\n';
                next_scan_char = read_char();
                break;
            }
            /* CR alone is replaced by LF */
            current_char = '\n';
        }

        /* Check for a C-style comment */
        if ((current_char == '/') & (next_scan_char == '*')) {
            current_char = ' ';
            next_scan_char = read_char();
            while (1) {
                if (next_scan_char == 0) {
                    fatal("Unterminated comment");
                }
                if (next_scan_char == '*') {
                    next_scan_char = read_char();
                    if (next_scan_char == '/') {
                        next_scan_char = read_char();
                        break;
                    }
                    continue;
                }
                // a carriage return followed by a line feed counts as one line
                // ending. otherwise, either a carriage return or a line feed
                // is a line ending.
                if (next_scan_char == '\r') {
                    current_line = (current_line + 1);
                    emit_char('\n');
                    next_scan_char = read_char();
                    if (next_scan_char == '\n') {
                        next_scan_char = read_char();
                    }
                    continue;
                }
                if (next_scan_char == '\n') {
                    current_line = (current_line + 1);
                    emit_char('\n');
                }
                next_scan_char = read_char();
            }
            continue;
        }

        /* Check for a C++-style comment */
        if ((current_char == '/') & (next_scan_char == '/')) {
            current_char = ' ';

            next_scan_char = read_char();
            while (1) {
                if (next_scan_char == 0) {
                    break;
                }
                /* Check for an escaped newline */
                if (next_scan_char == '\\') {
                    next_scan_char = read_char();
                    if (next_scan_char == '\r') {
                        next_scan_char = read_char();
                        if (next_scan_char == '\n') {
                            next_scan_char = read_char();
                            continue;
                        }
                        continue;
                    }
                    if (next_scan_char == '\n') {
                        next_scan_char = read_char();
                    }
                    continue;
                }
                /* Check for the end of the line */
                if ((next_scan_char == '\n') | (next_scan_char == '\r')) {
                    break;
                }
                next_scan_char = read_char();
            }
            if (next_scan_char == 0) {
                /* The file ended in a C++-style comment without a newline.
                 * Synthesize a newline. */
                next_scan_char = '\n';
            }
            break;
        }

        /* Check for lack of newline at end of file */
        if (next_scan_char == 0) {
            if ((current_char != 0) & ((current_char != '\n') & (current_char != '\r'))) {
                /* Insert a synthetic newline */
                next_scan_char = '\n';
                break;
            }
        }

        /* Done */
        break;
    }

    //printf("next done, char '%c'\n",current_char);
}

static void emit_string_or_character_literal(void) {

    // emit opening quote
    char quote = current_char;
    emit_char(current_char);
    next_char();

    // emit contents
    while (current_char != quote) {
        if ((current_char == '\n') | (current_char == 0)) {
            fatal("Unclosed single or double quote");
        }
        if (current_char == '\\') {
            emit_char(current_char);
            next_char();
            if ((current_char == '\n') | (current_char == 0)) {
                fatal("Unclosed single or double quote");
            }
        }
        emit_char(current_char);
        next_char();
    }

    // emit closing quote
    if ((current_char == '\n') | (current_char == 0)) {
        fatal("Unclosed single or double quote");
    }
    emit_char(current_char);
    next_char();
}



/*
 * Parsing helper functions
 */

/* When accumulating an identifier or expansion, we store it here */
static char* current_string;
static size_t current_string_capacity;

/**
 * Consumes any amount of horizontal whitespace, even if there is none.
 */
static void consume_optional_horizontal_whitespace(void) {
    while ((current_char == ' ') | (current_char == '\t')) {
        next_char();
    }
}

/**
 * Consumes at least one character of horizontal whitespace.
 */
static void consume_horizontal_whitespace(void) {
    if (!((current_char == ' ') | (current_char == '\t'))) {
        //printf("current char: %x\n", current_char);
        fatal("Expected horizontal whitespace");
    }
    consume_optional_horizontal_whitespace();
}

/**
 * Consumes whitespace up to and including one newline character.
 *
 * Raises a fatal error if a newline was not found.
 */
static void consume_end_of_line(void) {
    consume_optional_horizontal_whitespace();
    if (current_char != '\n') {
        fatal("Expected end of line");
    }
    next_char();
}

static bool is_identifier_char(int c, int first_char) {
    if (isalpha(c) | (c == '_')) {
        return true;
    }

    // We accept $ in identifiers as an extension.
    if (c == '$') {
        return true;
    }

    // Identifiers cannot start with digits.
    if (!first_char) {
        if (isdigit(c)) {
            return true;
        }
    }

    return false;
}

/**
 * Consumes an identifier, placing it (null-terminated) in current_string.
 */
static void consume_identifier(void) {
    size_t i;

    if (!is_identifier_char(current_char, true)) {
        fatal("Expected identifier");
    }

    *current_string = current_char;
    i = 1;
    while (1) {
        next_char();
        if (!is_identifier_char(current_char, false)) {
            break;
        }
        *(current_string + i) = current_char;
        i = (i + 1);
        if (i == current_string_capacity) {
            fatal("Identifier is too long.");
        }
    }

    *(current_string + i) = 0;
}

/**
 * Consumes a macro expansion string, placing it (null-terminated) in
 * current_string.
 */
static void consume_expansion(void) {
    size_t i;
    i = 0;
    while (1) {
        if (i == current_string_capacity) {
            fatal("Macro expansion is too long.");
        }

        if ((current_char == 0) | ((current_char == '\n') | (current_char == '\r'))) {
            break;
        }

        *(current_string + i) = current_char;
        i = (i + 1);
        next_char();
    }

    *(current_string + i) = 0;
}



/*
 * We store macros in a linked list.
 *
 * We'd like our macro struct to look something like this:
 *
 *     struct macro_t {
 *         struct macro_t* next;
 *         const char* name;
 *         const char* expansion;
 *         bool in_use;
 *      };
 *
 * Since we don't have structs, we'll just make it an array of four void
 * pointers.
 *
 * We insert new macros at the front of the linked list, that way if a macro is
 * redefined, we find the latest definition first. We don't bother to free()
 * replaced macros and #undef is not supported.
 */

int MACRO_NEXT;
int MACRO_NAME;
int MACRO_EXPANSION;
int MACRO_IN_USE;

static void macros_init(void) {
    MACRO_NEXT = 0;
    MACRO_NAME = 1;
    MACRO_EXPANSION = 2;
    MACRO_IN_USE = 3;
}

// The linked list of macros.
static void** macros;

static void macro_new(const char* name, const char* expansion) {
    if ((name == 0) | (expansion == 0)) {
        fatal("Out of memory.");
    }
    //printf("define macro %s %s\n", name, expansion);
    void** macro = (void**)malloc(sizeof(void*) * 4);
    *(macro + MACRO_NEXT) = (void*)macros;
    *(macro + MACRO_NAME) = (void*)name;
    *(macro + MACRO_EXPANSION) = (void*)expansion;
    *(macro + MACRO_IN_USE) = 0;
    macros = macro;
}

static void macro_delete(void** macro) {
    free(*(macro + MACRO_NAME));
    free(*(macro + MACRO_EXPANSION));
    free(macro);
}

static void** macro_find(const char* name) {
    void** macro = macros;
    while (macro != 0) {
        char* other_name;
        other_name = (char*)*(macro + MACRO_NAME);
        if (0 == strcmp(name, other_name)) {
            return macro;
        }
        macro = (void**)*(macro + MACRO_NEXT);
    }
    return 0;
}



/*
 * Include paths are also stored in a linked list.
 *
 * The struct for an include path would look something like this:
 *
 *     struct include_path_t {
 *         struct include_path_t* next;
 *         const char* path;
 *     };
 *
 * Like macros, we use an array of two void pointers. Unlike macros, we append
 * paths at the end so that priority is in the order specified on the
 * command-line.
 */

static void** include_paths;
static void** last_include_path;

static void include_path_new(const char* path) {
    if (strlen(path) == 0) {
        fatal("Empty include path.");
    }

    void** entry;
    entry = (void**)malloc(sizeof(void*) * 2);
    *entry = 0;
    *(entry + 1) = (void*)path;

    /* The first one goes at the head of the list */
    if (last_include_path == 0) {
        include_paths = entry;
    }
    /* Otherwise we append to the end */
    if (last_include_path != 0) {
        *last_include_path = entry;
    }

    last_include_path = entry;
}



/*
 * Include path searching
 */

static bool try_include(const char* prefix) {
    size_t prefix_length;
    prefix_length = strlen(prefix);

    char* suffix;
    size_t suffix_length;
    suffix = current_string;
    suffix_length = strlen(suffix);
    //printf("trying prefix %s suffix %s\n",prefix,suffix);

    // Check if the prefix has a trailing slash. If not we need to add one.
    // (We've checked in include_path_new() that the prefix is not empty.)
    int needs_slash;
    needs_slash = 1;
    if (*(prefix + (prefix_length - 1)) == '/') {
        needs_slash = 0;
    }

    // Assemble the path
    char* path;
    size_t path_length;
    path = malloc((prefix_length + suffix_length) + 2);
    memcpy(path, prefix, prefix_length);
    path_length = prefix_length;
    if (needs_slash) {
        *(path + path_length) = '/';
        path_length = (path_length + 1);
    }
    memcpy(path + path_length, suffix, suffix_length);
    path_length = (path_length + suffix_length);
    *(path + path_length) = 0;

    // Try to open the file
    FILE* file;
    file = fopen(path, "r");
    if (file == 0) {
        free(path);
        return false;
    }

    fputs("#pragma onramp file push\n", output_file);
    preprocess(path, file);
    fputs("#pragma onramp file pop\n", output_file);

    // Emit a line directive for the parent file to get us back where we were.
    // (The parent file is NULL if we're processing `-include`.)
    if (current_filename != 0) {
        emit_line_directive();
    }

    fclose(file);
    free(path);
    return true;
}

static int find_slash(const char* str) {

    // The parent filename is NULL when using `-include`.
    if (str == 0) {
        return -1;
    }

    int slash_index;
    slash_index = -1;

    int i;
    i = 0;

    while (1) {
        char c;
        c = *(str + i);
        if (c == 0) {
            break;
        }
        if (c == '/') {
            slash_index = i;
        }
        i = (i + 1);
    }

    return slash_index;
}

static void search_include(char quote_style) {
    //printf("search include %s\n", current_string);

    // The path is in current_string. If it starts with /, it's an absolute
    // path so we just open that file directly.
    if (*current_string == '/') {
        FILE* file;
        file = fopen(current_string, "r");
        if (file == 0) {
            fatal("Failed to open include file with absolute path.");
        }
        preprocess(current_string, file);
        return;
    }

    // Only check the path of the current file if we're using quotes, not angle
    // brackets.
    if (quote_style == '"') {

        // Get the path of the current file.
        int slash_index;
        slash_index = find_slash(current_filename);

        // If there's no path, try the current path
        if (slash_index == -1) {
            if (try_include(".")) {
                return;
            }
        }

        // If there is a path, check there.
        if (slash_index != -1) {
            char* prefix = strndup(current_filename, slash_index);
            int found = try_include(prefix);
            free(prefix);
            if (found) {
                return;
            }
        }
    }

    // Search through our include paths.
    void** entry;
    entry = include_paths;
    while (entry != 0) {
        //printf("entry %s\n", (const char*)*(entry + 1));
        if (try_include((const char*)*(entry + 1))) {
            return;
        }
        entry = (void**)*entry;
    }

    fatal("include file not found.");
}



/*
 * Parsing
 */

static void handle_line(void) {
    fatal("#line preproc not implemented");
}

static void handle_include(void) {
    /* A lack of space between #include and the filename is apparently allowed. */
    consume_optional_horizontal_whitespace();

    /* The end quote style must match the start quote style */
    char end_char;
    end_char = 0;
    if (current_char == '"') {
        end_char = '"';
    }
    if (current_char == '<') {
        end_char = '>';
    }
    if (end_char == 0) {
        fatal("include must be followed by < or a double quote");
    }
    next_char();

    /* Scan until we find the end, accumulating into current_string */
    size_t i;
    i = 0;
    while (current_char != end_char) {
        // check for end of line
        if (((current_char == 0xA) | (current_char == 0xD)) | (current_char == -1)) {
            fatal("Unclosed include");
        }
        *(current_string + i) = current_char;
        i = (i + 1);
        next_char();
        if (i == current_string_capacity) {
            fatal("include filename is too long.");
        }
    }
    next_char();

    // Finish parsing
    if (i == 0) {
        fatal("Expected a filename for include.");
    }
    *(current_string + i) = 0;
    consume_end_of_line();

    search_include(end_char);
}

static void handle_define(void) {
    consume_horizontal_whitespace();
    consume_identifier();
    char* name;
    name = strdup(current_string);

    // If the name is followed by an open parenthesis, this is a function-like
    // macro.
    if (current_char == '(') {
        fatal("Function-like macros are not supported by the omC preprocessor.");
    }

    // If we have a newline without horizontal whitespace, there's no expansion
    // string.
    if (current_char == '\n') {
        macro_new(name, strdup(""));
        return;
    }

    // Otherwise we require horizontal whitespace before the expansion. The
    // expansion might still be blank.
    consume_horizontal_whitespace();
    consume_expansion();
    char* expansion;
    expansion = strdup(current_string);
    consume_end_of_line();
    macro_new(name, expansion);
}

static void handle_conditional(int predicate) {
    if (predicate) {
        /* The #ifdef/#ifndef is true. Keep parsing. */
        depth = (depth + 1);
        return;
    }

    /* The #ifdef/#ifndef conditional is false. We have to consume everything
     * until the matching #endif, making sure to track nested #ifdef/#ifndefs. */
    int false_depth;
    false_depth = 0;
    while (1) {
        if (current_char == '#') {
            next_char();

            consume_optional_horizontal_whitespace();

            /* Any old nonsense is allowed in untaken conditional branches (and
             * in particular we don't want to fail on `##` token pasting and
             * other things.) If we don't have an identifier we skip. */
            if (!is_identifier_char(current_char, true)) {
                next_char();
                continue;
            }

            /* We only care about #ifdef/#ifndef and #endif. */
            consume_identifier();
            if ((0 == strcmp(current_string, "ifdef")) | (0 == strcmp(current_string, "ifndef"))) {
                false_depth = (false_depth + 1);
                continue;
            }
            if (0 == strcmp(current_string, "endif")) {
                if (false_depth == 0) {
                    /* Done. */
                    return;
                }
                false_depth = (false_depth - 1);
                continue;
            }

            /* No other directives matter. (In fact the C spec requires that we
             * ignore whatever we find in preprocessor directives on untaken
             * conditional branches.) */
            continue;
        }

        /* If we've reached the end of the file, we're missing an #endif */
        if (current_char == 0) {
            fatal("Unclosed if; expected endif before end of file");
        }

        /* We don't care about anything else. */
        next_char();
    }
}

static void handle_ifdef(void) {
    consume_horizontal_whitespace();
    consume_identifier();
    consume_end_of_line();
    handle_conditional(macro_find(current_string) != 0);
}

static void handle_ifndef(void) {
    consume_horizontal_whitespace();
    consume_identifier();
    consume_end_of_line();
    handle_conditional(macro_find(current_string) == 0);
}

static void handle_endif(void) {
    consume_end_of_line();
    if (depth == 0) {
        fatal("endif without if");
    }
    depth = (depth - 1);
}

static void expand_macro(void** macro);

static void expand_identifier(const char* identifier) {
    void** macro;
    macro = macro_find(identifier);

    /* See if it's a macro */
    if (macro != 0) {
        //printf("expand macro %s\n",identifier);
        expand_macro(macro);
        return;
    }

    /* Not a macro. Just emit it. */
    while (1) {
        if (*identifier == 0) {
            break;
        }
        emit_char(*identifier);
        identifier = (identifier + 1);
    }
}

static void expand_macro(void** macro) {
    const char* expansion;
    size_t i;
    expansion = (const char*)*(macro + MACRO_EXPANSION);
    i = 0;

    /* Check and set in_use flag */
    if (*(macro + MACRO_IN_USE) != 0) {
        fatal("Recursive macros are not allowed in omC.");
    }
    *(macro + MACRO_IN_USE) = (void*)1;

    while (1) {
        char c;
        c = *(expansion + i);

        /* Check for end of string */
        if (c == 0) {
            break;
        }

        /* Collect and handle identifiers */
        if (is_identifier_char(c, true)) {
            size_t j;
            j = 0;
            while (1) {
                *(current_string + j) = c;
                i = (i + 1);
                j = (j + 1);
                if (j == current_string_capacity) {
                    /* This shouldn't be possible because we're expanding a
                     * string that was stored in current_string in the first
                     * place. We check anyway for safety. */
                    fatal("Internal error: expand is too long");
                }
                c = *(expansion + i);
                if (!is_identifier_char(c, false)) {
                    break;
                }
            }
            *(current_string + j) = 0;
            expand_identifier(current_string);
            continue;
        }

        /* Emit numbers as-is */
        if (isdigit(c)) {
            while (isalnum(c)) {
                emit_char(c);
                i = (i + 1);
                c = *(expansion + i);
            }
            continue;
        }

        /* It's not an identifier or number */
        //printf("emit char %c %i\n", c, (int)i);
        emit_char(c);
        i = (i + 1);
    }

    /* Clear in_use flag */
    *(macro + MACRO_IN_USE) = (void*)0;
}

static void preprocess(const char* new_filename, FILE* new_file) {

    // store previous file on the call stack
    FILE* old_file;
    char* old_filename;
    int old_line;
    int old_depth;
    char old_current_char;
    char old_next_scan_char;
    old_file = current_file;
    old_filename = current_filename;
    old_line = current_line;
    old_depth = depth;
    old_current_char = current_char;
    old_next_scan_char = next_scan_char;

    // setup the new file
    depth = 0;
    current_filename = strdup(new_filename);
    current_file = new_file;
    current_line = 1;
    current_char = 0;
    next_scan_char = 0;
    emit_line_directive();
    next_char();
    next_char();

    while (1) {

        /* Handle preprocessor directives (note that we don't bother to check
         * that we are at the beginning of a line) */
        if (current_char == '#') {
            next_char();
            consume_optional_horizontal_whitespace();

            if (current_char == '\n') {
                /* Empty directive is allowed. */
                next_char();
                continue;
            }

            consume_identifier();
            if (0 == strcmp(current_string, "line")) {
                handle_line();
                continue;
            }
            if (0 == strcmp(current_string, "include")) {
                handle_include();
                continue;
            }
            if (0 == strcmp(current_string, "define")) {
                handle_define();
                continue;
            }
            if (0 == strcmp(current_string, "ifdef")) {
                handle_ifdef();
                continue;
            }
            if (0 == strcmp(current_string, "ifndef")) {
                handle_ifndef();
                continue;
            }
            if (0 == strcmp(current_string, "endif")) {
                handle_endif();
                continue;
            }
            if (0 == strcmp(current_string, "error")) {
                fatal("#error directive reached.");
            }
            fatal("Unrecognized preprocessor directive.");
        }

        /* Expand identifiers */
        if (is_identifier_char(current_char, true)) {
            consume_identifier();
            expand_identifier(current_string);
            continue;
        }

        /* Emit numbers as-is */
        if (isdigit(current_char)) {
            while (isalnum(current_char)) {
                emit_char(current_char);
                next_char();
            }
            continue;
        }

        /* Emit string and character literals as-is */
        if ((current_char == '\'') | (current_char == '"')) {
            emit_string_or_character_literal();
            continue;
        }

        /* Stop at end of input */
        if (current_char == 0) {
            break;
        }

        /* Emit all other characters as-is (except for line endings, which the
         * character scanner emits internally) */
        if (current_char != '\n') {
            emit_char(current_char);
        }
        next_char();
    }

    if (depth != 0) {
        fatal("Unclosed if; expected endif before end of file");
    }

    // restore previous file
    current_file = old_file;
    free(current_filename);
    current_filename = old_filename;
    current_line = old_line;
    depth = old_depth;
    current_char = old_current_char;
    next_scan_char = old_next_scan_char;
}



/*
 * Initialization
 */

static void initialize(void) {
    macros_init();

    current_string_capacity = 256;
    current_string = malloc(current_string_capacity);
}

static void define_default_macros(void) {
    if (!opt_nostddef) {
        // Note that we don't define __onramp__. It is defined by the cc driver.
        macro_new(strdup("__onramp_cpp__"), strdup("1"));
        macro_new(strdup("__onramp_cpp_omc__"), strdup("1"));
    }
}

static void deinitialize(void) {

    // free macros
    void** macro = macros;
    while (macro != 0) {
        void** next = (void**)*(macro + MACRO_NEXT);
        macro_delete(macro);
        macro = next;
    }

    // free include paths
    void** entry = include_paths;
    while (entry != 0) {
        void** next = (void**)*entry;
        free(entry);
        entry = next;
    }

    free(current_string);
}

static void parse_command_line(int argc, const char** argv) {
    int i;
    i = 0;

    while (1) {
        i = (i + 1);
        if (i == argc) {
            break;
        }

        const char* arg;
        int len;

        arg = *(argv + i);
        len = strlen(arg);

        /* Macro definition on command-line */
        if ((len >= 2) & (0 == memcmp(arg, "-D", 2))) {
            const char* name;
            const char* expansion;

            if (len == 2) {
                /* -D followed by a space; the macro is the next argument */
                i = (i + 1);
                if (i == argc) {
                    fatal("-D must be followed by a macro name.");
                }
                name = *(argv + i);
            }
            if (len != 2) {
                /* -D with no space; the macro is attached. */
                name = (arg + 2);
            }

            /* See if there's an = sign */
            expansion = strchr(name, '=');
            if (expansion != 0) {
                name = strndup(name, (intptr_t)expansion - (intptr_t)name); /* extract the macro name */
                expansion = (expansion + 1); /* skip the = */
            }
            if (expansion == 0) {
                expansion = "";
            }

            macro_new(name, strdup(expansion));
            continue;
        }

        /* Include path on command-line */
        if ((len >= 2) & (0 == memcmp(arg, "-I", 2))) {
            const char* path;

            if (len == 2) {
                /* -I followed by a space; the path is the next argument */
                i = (i + 1);
                if (i == argc) {
                    fatal("-I must be followed by a path.");
                }
                path = *(argv + i);
            }
            if (len != 2) {
                /* -I with no space; the path is attached. */
                path = (arg + 2);
            }

            //printf("-I path %s\n", path);
            include_path_new(path);
            continue;
        }

        /* Output file */
        if ((len >= 2) & (0 == memcmp(arg, "-o", 2))) {
            if (output_filename != 0) {
                fatal("Only one `-o` output filename can be specified.");
            }

            if (len == 2) {
                /* -o followed by a space; the path is the next argument */
                i = (i + 1);
                if (i == argc) {
                    fatal("`-o` must be followed by a filename.");
                }
                output_filename = *(argv + i);
            }
            if (len != 2) {
                /* -o with no space; the filename is attached. */
                output_filename = (arg + 2);
            }
            continue;
        }

        /* Force include */
        if (0 == strcmp(arg, "-include")) {
            if (force_include_filename != 0) {
                fatal("Only one `-include` file can be specified.");
            }
            i = (i + 1);
            if (i == argc) {
                fatal("`-include` must be followed by a filename.");
            }
            force_include_filename = *(argv + i);
            continue;
        }

        /* Other options */
        if (0 == strcmp(arg, "-nostddef")) {
            opt_nostddef = true;
            continue;
        }
        if (0 == strcmp(arg, "-dM")) {
            opt_dump_macros = true;
            continue;
        }

        /* Input file */
        if (input_filename != 0) {
            fatal("Only one input filename can be specified.");
        }
        input_filename = arg;
    }

    if (input_filename == 0) {
        fatal("An input file is required.");
    }
    if (output_filename == 0) {
        fatal("An output file is required.");
    }
}

void do_force_include(void) {
    if (force_include_filename == 0) {
        return;
    }

    // The filename needs to be in current_string.
    if (strlen(force_include_filename) > (current_string_capacity - 1)) {
        fatal("`-include` filename is too long.");
    }
    strcpy(current_string, force_include_filename);

    // We use the normal search paths for `-include` files.
    search_include('"');
}

void dump_macros(void) {
    void** macro = macros;
    while (macro != 0) {
        char* name = (char*)*(macro + MACRO_NAME);
        char* expansion = (char*)*(macro + MACRO_EXPANSION);
        fputs("#define ", stdout);
        fputs(name, stdout);
        putchar(' ');
        puts(expansion);
        macro = (void**)*(macro + MACRO_NEXT);
    }
}

int main(int argc, const char** argv) {
    #ifdef __onramp_cpp_strip__
    true = 1;
    #endif

    initialize();
    parse_command_line(argc, argv);
    define_default_macros();

    output_file = fopen(output_filename, "w");
    if (output_file == 0) {
        fatal("Failed to open output file");
    }

    /* Preprocess `-include` file */
    do_force_include();

    /* Preprocess input file */
    FILE* input_file = fopen(input_filename, "r");
    if (input_file == 0) {
        fatal("Failed to open input file");
    }
    preprocess(input_filename, input_file);

    fclose(input_file);
    fclose(output_file);

    if (opt_dump_macros) {
        dump_macros();
    }

    deinitialize();
    return 0;
}
