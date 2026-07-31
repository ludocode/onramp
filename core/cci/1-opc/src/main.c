/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compile.h"
#include "emit.h"
#include "lexer.h"
#include "parse-stmt.h"
#include "parse-decl.h"
#include "parse-expr.h"
#include "types.h"
#include "locals.h"
#include "global.h"

static void usage(const char* name) {
    fputs("\nUsage: ", stderr);
    fputs(name, stderr);
    fputs(" <input_file> -o <output_file>\n", stderr);
    exit(1);
}

int main(int argc, const char** argv) {
    #ifndef __onramp__
    setvbuf(stdout, NULL, _IONBF, 0); // TODO debugging
    #endif

    // parse command-line options
    const char* input_filename = NULL;
    const char* output_filename = NULL;
    int i = 1;
    while (i < argc) {

        // output
        if (0 == strcmp("-o", *(argv + i))) {
            if (output_filename != NULL) {
                fputs("ERROR: -o cannot be specified twice.", stderr);
                usage(*argv);
            }
            i = (i + 1);
            if (i == argc) {
                fputs("ERROR: -o must be followed by a filename.", stderr);
                usage(*argv);
            }
            output_filename = *(argv + i);
            i = (i + 1);
            continue;
        }

        // misc options
        if (0 == strcmp("-mabi=bootstrap", *(argv + i))) {
            // ignored, this stage only supports the bootstrap abi
            i = (i + 1);
            continue;
        }
        if (0 == strcmp("-O", *(argv + i))) {
            // ignored, optimization is done by cg/0
            i = (i + 1);
            continue;
        }
        if (0 == strcmp("-g", *(argv + i))) {
            // ignored, we always emit debug info
            i = (i + 1);
            continue;
        }

        // unrecognized option
        if (**(argv + i) == '-') {
            fputs("ERROR: Unsupported option: `", stderr);
            fputs(*(argv + i), stderr);
            fputc('`', stderr);
            usage(*argv);
        }

        // input
        if (input_filename != NULL) {
            fputs("ERROR: Only one input filename can be provided.", stderr);
            usage(*argv);
        }
        input_filename = *(argv + i);
        i = (i + 1);
    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage(*argv);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage(*argv);
    }

    global_setup();
    emit_setup(output_filename);
    lexer_setup(input_filename);
    types_setup();
    locals_setup();
    compile_setup();
    parse_stmt_setup();
    parse_decl_setup();
    parse_expr_setup();

    while (lexer_type != lexer_type_end) {
        parse_global();
    }

    parse_expr_teardown();
    parse_decl_teardown();
    parse_stmt_teardown();
    compile_teardown();
    locals_teardown();
    types_teardown();
    lexer_teardown();
    emit_teardown();
    global_teardown();

    return EXIT_SUCCESS;
}
