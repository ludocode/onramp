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

#include <stdlib.h>

#include "libo-string.h"
#include "parse_expr.h"
#include "parse_decl.h"
#include "parse_stmt.h"
#include "strings.h"
#include "block.h"
#include "lexer.h"
#include "emit.h"
#include "token.h"
#include "common.h"
#include "scope.h"
#include "options.h"

static const char* input_filename;
static const char* output_filename;

static void usage(const char* name) {
    fprintf(stderr, "\nUsage: %s <input_file> -o <output_file>\n", name);
    _Exit(1);
}

static void parse_command_line(char** argv) {
    const char* executable_name = *argv++;

    while (*argv) {

        // output filename
        if (0 == strcmp("-o", *argv)) {
            if (output_filename != NULL) {
                fputs("ERROR: -o cannot be specified twice.\n", stderr);
                usage(executable_name);
            }
            if (!*++argv) {
                fputs("ERROR: -o must be followed by a filename.\n", stderr);
                usage(executable_name);
            }
            output_filename = *argv++;
            continue;
        }

        // other options
        if (options_parse(*argv)) {
            ++argv;
            continue;
        }

        // unrecognized option
        if (**argv == '-') {
            fprintf(stderr, "ERROR: Unsupported option: %s", *argv);
            usage(executable_name);
        }

        // input filename
        if (input_filename != NULL) {
            fputs("ERROR: Only one input filename can be provided.", stderr);
            usage(*argv);
        }
        input_filename = *argv++;

    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage(executable_name);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage(executable_name);
    }
}

int main(int argc, char** argv) {
    string_table_init();
    options_init();

    parse_command_line(argv);
    options_resolve();

    strings_init();
    scope_global_init();
    parse_decl_init();
    parse_expr_init();
    parse_stmt_init();
    emit_init(output_filename);
    lexer_init(input_filename);

    while (lexer_token->type != token_type_end) {
        parse_global();
    }

    scope_emit_tentative_definitions();

    lexer_destroy();
    emit_destroy();
    parse_stmt_init();
    parse_expr_init();
    parse_decl_init();
    scope_global_destroy();
    strings_destroy();

    options_destroy();
    string_table_destroy();
    return 0;
}
