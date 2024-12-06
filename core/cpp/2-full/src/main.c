/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libo-error.h"
#include "libo-vector.h"
#include "strings.h"
#include "emit.h"
#include "preprocess.h"
#include "stream.h"
#include "lexer.h"
#include "directive.h"
#include "macro.h"

static const char* executable_name;
static const char* output_filename;
static const char* input_filename;
//static vector_t force_includes; // vector of token_t* strings

static void usage(void) {
    fprintf(stderr, "\nUsage: %s <input_file> -o <output_file>\n", executable_name);
    fprintf(stderr, "    -D<name>=<value>      -- define macro\n");
    fprintf(stderr, "    -I<path>              -- add include path\n");
    fprintf(stderr, "    -include <filename>   -- force include file\n");
    fprintf(stderr, "    see usage guide for more\n");
    _Exit(1);
}

/**
 * Returns the value associated with a single-letter command-line option,
 * incrementing the argument pointer past it.
 *
 * Lots of single-letter options allow an optional space between the option and
 * value, for example `-I /path/` vs. `-I/path/`. This function checks for
 * both.
 *
 * It returns the value if the option matches, NULL if it doesn't, and raises a
 * fatal error if the option matches but the value is missing. argv is
 * incremented past the consumed arguments.
 */
static char* single_letter_option_value(char option, char*** argv) {
    if (***argv != '-' || (**argv)[1] != option)
        return NULL;
    if ((**argv)[2] != 0)
        return *(*argv)++ + 2;
    char* value = *++*argv;
    if (value == NULL || *value == 0) {
        fprintf(stderr, "ERROR: Command-line option `-%c` must be followed by a value.\n", option);
        usage();
    }
    ++*argv;
    return value;
}

/*
 * Parses a -D command-line option.
 *
 * From my tests with GCC and other compilers, they seem to just scan the
 * argument for the first `=`. If found, they replace the character with a
 * space; if not, they append a space and a 1 onto the end of the argument.
 *
 * They then pass the modified argument through the directive parser as though
 * it was prefixed with "#define ". Anything after a newline is ignored
 * (including the appended " 1" if no `=` was found.)
 *
 * The = does not need to separate the arguments, cannot be escaped, and is
 * replaced with a space even if it would otherwise form part of a token (like
 * `==`.)
 *
 * So you can do bizarre things like this:
 */
 // '-DFOO\=n'                 ->    #define FOO \ n
 // '-DFOO(=a,/*test*/b)b,a'   ->    #define FOO(a,b) b,a
 // '-DFOO(a,b)b,1=2'          ->    #define FOO(a,b) b,1 2
 // '-DFOO(a/*,*/,b)==1'       ->    #define FOO(a,b) =1
 // '-DFOO(a/*)*/)123'         ->    #define FOO(a) 123 1
 // '-DFOO%3'$'\n'2            ->    #define FOO %3
 //TODO move the above into unit test cases
/*
 * Clang and TinyCC appear to have replicated this behaviour exactly. For now
 * we do too because it's trivial to implement.
 *
 * It might be better to add some restrictions later to at least require the
 * `=` to separate the arguments and expansion. This would also be extremely
 * easy to implement. We can otherwise preserve the GNU behaviour under
 * `-fgnu-extensions`.
 */
static void command_line_define(char* original) {

    // This implements the behaviour described above. If we have an `=`, we
    // replace it with a space; otherwise we append " 1".
    size_t length = strlen(original);
    char* equal = memchr(original, '=', length);
    char* modified; // not null-terminated
    if (equal) {
        // modify in-place
        modified = original;
        *equal = ' ';
    } else {
        // allocate copy
        modified = malloc(length + 2);
        memcpy(modified, original, length);
        modified[length] = ' ';
        modified[length + 1] = '1';
        length += 2;
    }

    // Parse it
    lexer_t* lexer = lexer_new_bytes(STR_COMMAND_LINE, modified, length);
    stream_t stream;
    stream_init_lexer(&stream, lexer);
    macro_define(&stream);
    stream_destroy(&stream);
    lexer_delete(lexer);

    if (modified != original) {
        free(modified);
    }
}

static void parse_command_line(char** argv) {
    executable_name = *argv++;

    while (*argv) {
        char* value;

        // output filename
        value = single_letter_option_value('o', &argv);
        if (value) {
            output_filename = value;
            continue;
        }

        // macro definition
        value = single_letter_option_value('D', &argv);
        if (value) {
            command_line_define(value);
            continue;
        }

        // include path
        value = single_letter_option_value('I', &argv);
        if (value) {
            preprocess_add_include_path(value);
            continue;
        }

        // force-include file
        if (0 == strcmp(*argv, "-include")) {
            if (!*++argv) {
                fputs("ERROR: Command-line option `-include` must be followed by a filename.\n", stderr);
                usage();
            }
            preprocess_add_force_include(*argv++);
            continue;
        }

        // unrecognized option
        if (**argv == '-') {
            fprintf(stderr, "ERROR: Unsupported command-line option: %s", *argv);
            usage();
        }

        // input filename
        if (input_filename != NULL) {
            fputs("ERROR: Only one input filename can be provided.", stderr);
            usage();
        }
        input_filename = *argv++;

    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage();
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage();
    }
}

int main(int argc, char** argv) {
    /*
    printf("command line:");
    for (int i = 0; i < argc; ++i) printf(" %s", argv[i]);
    printf("\n");
    */

    string_table_init();
    strings_setup();
    lexer_setup();
    token_setup();
    macro_setup();
    directive_setup();
    emit_setup();
    preprocess_setup();

    parse_command_line(argv);
    assert(input_filename);
    assert(output_filename);

    // TODO move these somewhere else
    macro_define_builtins();

    emit_open(output_filename);
    preprocess(input_filename);

    preprocess_teardown();
    emit_teardown();
    directive_teardown();
    macro_teardown();
    token_teardown();
    lexer_teardown();
    strings_teardown();
    string_table_destroy();
}
