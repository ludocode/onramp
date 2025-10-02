/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Fraser Heavy Software
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

/**
 * This is the main source file for the first stage code generator (cg/0).
 *
 * We haven't built our second stage linker yet so we have to put everything in
 * one translation unit. For this reason most of our code is in include files.
 */

// Some include files themselves include others. We don't list them all here
// to save some processing time since our second stage preprocessor doesn't
// have an include guard optimization. (It would be slightly faster if we just
// omitted the include guards and only used #include here but that would be
// less idiomatic; we want our code to be as "normal C" as possible.)
#include "parse.h"
#include "libo-error.h"
#include "optimize.h"

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

static void emit_function(void) {
    fputc(function_prefix, output_file);
    fputs(function_name, output_file);
    fputc('\n', output_file);

    size_t i = 0;
    while (i != instructions_count) {
        instruction_emit(*(instructions + i));
        i = (i + 1);
    }

    fputs("\n\n", output_file);
}

int main(int argc, char** argv) {

    // (this code is copied from cci/1)

    // parse command-line options
    char* input_filename = NULL;
    char* output_filename = NULL;
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

        // unrecognized option
        if (**(argv + i) == '-') {
            fputs("ERROR: Unsupported option: ", stderr);
            fputs(*(argv + i), stderr);
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

    opcode_setup();
    instruction_setup();
    register_setup();
    parse_setup();
    optimize_setup();

    open_output(output_filename);
    open_input(input_filename);
    current_filename = input_filename;
    current_line = 1;
    read_char();

    while (parse_function()) {

        // Replace push/pop pairs with movs to unused registers
        optimize_push_pop();

        // Scan forward: propagate constants and registers, eliminate unreachable code
        optimize_forward();

        // Eliminate any blocks that are unused
        optimize_blocks();

        // Scan backward: perform dead store elimination, register renaming
        optimize_backward();

        emit_function();
        parse_clear();
    }

    fclose(input_file);
    fclose(output_file);

    optimize_teardown();
    parse_teardown();
    register_teardown();
    instruction_teardown();
    opcode_teardown();

    return 0;
}
