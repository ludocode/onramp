/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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

#include "options.h"

#include <stdlib.h>

#include "common.h"
#include "lexer.h"
#include "macro.h"
#include "stream.h"
#include "strings.h"

const char* options_executable_name;
const char* options_output_filename;
const char* options_input_filename;
bool options_nostddef;
vector_t options_include_paths;
vector_t options_force_includes;

void options_setup(void) {
    vector_init(&options_include_paths);
    vector_init(&options_force_includes);
}

void options_teardown(void) {
    destroy_string_vector(&options_force_includes);
    destroy_string_vector(&options_include_paths);
}

static void usage(void) {
    fprintf(stderr, "\nUsage: %s <input_file> -o <output_file>\n", options_executable_name);
    fprintf(stderr, "    -D<name>=<value>      -- define macro\n");
    fprintf(stderr, "    -I<path>              -- add include path\n");
    fprintf(stderr, "    -include <filename>   -- force include file\n");
    fprintf(stderr, "    see usage guide for more\n");
    exit(1);
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
static char* options_single_letter(char option, char*** argv) {
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
static void options_define(char* original) {

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
    lexer_current = lexer_new_bytes(STR_COMMAND_LINE, modified, length);
    stream_t stream;
    stream_init(&stream, true, NULL);
    macro_parse(&stream);
    stream_destroy(&stream);
    lexer_delete(lexer_current);

    if (modified != original) {
        free(modified);
    }
}

/**
 * We do a pre-scan for certain options. In particular, we need to add builtin
 * macros before other options so that -D and -U can be used on them the way
 * the user expects, so we need to scan for -nostddef first to see if we should
 * define them.
 */
static void options_pre_parse(char** argv) {
    while (*argv) {
        if (0 == strcmp(*argv, "-nostddef")) {
            options_nostddef = true;
            break;
        }
        ++argv;
    }

    macro_define_builtins();
}

void options_parse(char** argv) {
    options_executable_name = *argv++;

    options_pre_parse(argv);

    while (*argv) {
        char* value;

        // output filename
        value = options_single_letter('o', &argv);
        if (value) {
            options_output_filename = value;
            continue;
        }

        // macro definition
        value = options_single_letter('D', &argv);
        if (value) {
            options_define(value);
            continue;
        }

        // include path
        value = options_single_letter('I', &argv);
        if (value) {
            vector_append(&options_include_paths, string_intern_cstr(value));
            continue;
        }

        // force-include file
        if (0 == strcmp(*argv, "-include")) {
            if (!*++argv) {
                fputs("ERROR: Command-line option `-include` must be followed by a filename.\n", stderr);
                usage();
            }
            vector_append(&options_force_includes, string_intern_cstr(*argv++));
            continue;
        }

        // pre-scan options already handled
        if (0 == strcmp(*argv, "-nostddef")) {
            ++argv;
            continue;
        }

        // unrecognized option
        if (**argv == '-') {
            fprintf(stderr, "ERROR: Unsupported command-line option: %s", *argv);
            usage();
        }

        // input filename
        if (options_input_filename != NULL) {
            fputs("ERROR: Only one input filename can be provided.", stderr);
            usage();
        }
        options_input_filename = *argv++;
    }

    if (options_input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage();
    }
    if (options_output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage();
    }
}
