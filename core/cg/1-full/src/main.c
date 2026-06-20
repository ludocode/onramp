/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025-2026 Fraser Heavy Software
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
 * This is the entry point of the second stage code generator.
 *
 * For now this is mostly a copy of the main file of the first stage. It just
 * reads command line arguments and opens and closes files.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "analyze.h"
#include "block.h"
#include "common.h"
#include "emit.h"
#include "libo-error.h"
#include "libo-vector.h"
#include "location.h"
#include "opcode.h"
#include "optimize.h"
#include "parse.h"
#include "symbol.h"
#include "temporary.h"
#include "transform.h"
#include "variable.h"

static void usage(const char* name) {
    fputs("\nUsage: ", stderr);
    fputs(name, stderr);
    fputs(" <input_file> -o <output_file>\n", stderr);
    exit(1);
}

static void open_output(const char* output_filename) {
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        fatal("ERROR: Failed to open output file.");
    }
}

static void open_input(const char* input_filename) {
    input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        fatal("ERROR: Failed to open input file.");
    }
}

int main(int argc, char** argv) {

    // (this code is copied from cci/1)

    // parse command-line options
    char* input_filename = NULL;
    char* output_filename = NULL;
    int i = 1;
    while (i < argc) {

        // output
        if (0 == strcmp("-o", argv[i])) {
            if (output_filename != NULL) {
                fputs("ERROR: -o cannot be specified twice.", stderr);
                usage(argv[0]);
            }
            if (++i == argc) {
                fputs("ERROR: -o must be followed by a filename.", stderr);
                usage(argv[0]);
            }
            output_filename = argv[i++];
            continue;
        }

        // optimize
        if (0 == strcmp("-O", argv[i])) {
            optimize = true;
            ++i;
            continue;
        }

        // unrecognized option
        if (argv[i][0] == '-') {
            fputs("ERROR: Unsupported option: ", stderr);
            fputs(argv[i], stderr);
            usage(argv[0]);
        }

        // input
        if (input_filename != NULL) {
            fputs("ERROR: Only one input filename can be provided.", stderr);
            usage(argv[0]);
        }
        input_filename = argv[i++];
    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage(argv[0]);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage(argv[0]);
    }

    string_setup();
    current_filename_string_setup();
    location_setup();
    opcode_setup();
    variables_setup();
    temporaries_setup();
    blocks_setup();

    // setup files
    open_output(output_filename);
    open_input(input_filename);
    parse_setup(input_filename);
    emit_setup();

    // parse, optimize, codegen and emit symbols
    for (;;) {
        symbol_t* symbol = try_parse_symbol();
        if (!symbol) {
            break;
        }

        if (vector_is_empty(symbol->blocks)) {
            fatal("TODO empty symbol");
        }

        // perform initial conversions
        convert_vars(symbol);
        convert_entry(symbol);

        // TODO optimizations

        // convert to assembly
        analyze_liveness(symbol);
        convert_control_flow(symbol);

        emit_symbol(symbol);
        symbol_delete(symbol);

        // reset state
        pass_id = 0;
        temporaries_clear();
        variables_clear();
        blocks_clear();
    }

    parse_teardown();
    fclose(input_file);
    fclose(output_file);

    emit_teardown();
    blocks_teardown();
    temporaries_teardown();
    variables_teardown();
    opcode_teardown();
    location_teardown();
    current_filename_string_teardown();
    string_teardown();

    return 0;
}
