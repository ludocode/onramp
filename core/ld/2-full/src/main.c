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
 * This is the Onramp final linker.
 *
 * It supports all Onramp linker features, performs garbage collection of
 * unused symbols, and gives proper error messages with line numbers.
 */

// TODO asprintf() is nonstandard (is it in C23?)
#define _GNU_SOURCE

#include <sys/stat.h>

#include "common.h"
#include "symbol.h"
#include "label.h"
#include "parse.h"
#include "emit.h"

static const char** input_filenames;
static size_t input_filenames_count;
static size_t input_filenames_capacity;

static const char* output_filename;
static const char* wrap_header;

static void parse_args(const char** argv) {
    input_filenames_count = 0;
    input_filenames_capacity = 8;
    input_filenames = malloc(sizeof(char*) * input_filenames_capacity);
    if (input_filenames == NULL) {
        fatal("Out of memory.");
    }

    // skip program name
    ++argv;

    while (*argv != NULL) {

        // optimization
        if (0 == strcmp(*argv, "-O")) {
            option_optimize = true;
            ++argv;
            continue;
        }

        // debug info
        if (0 == strcmp(*argv, "-g")) {
            option_debug = true;
            ++argv;
            continue;
        }

        // wrap header
        if (0 == strcmp(*argv, "-wrap-header")) {
            if (*++argv == 0) {
                fatal("-wrap-header must be followed by an output file.");
            }
            if (wrap_header != NULL) {
                fatal("Only one -wrap-header can be specified.");
            }
            wrap_header = *argv++;
            continue;
        }

        // output file
        if (0 == strcmp(*argv, "-o")) {
            if (*++argv == 0) {
                fatal("-o must be followed by an output file.");
            }
            if (output_file != NULL) {
                fatal("Only one -o output file can be specified.");
            }
            //printf("found %s as output\n",*argv);
            output_filename = *argv++;
            continue;
        }

        // otherwise it's an input file
        if (input_filenames_count == input_filenames_capacity) {
            size_t new_capacity = input_filenames_capacity * 2;
            if (new_capacity <= input_filenames_capacity) {
                fatal("Out of memory.");
            }
            input_filenames = realloc(input_filenames, sizeof(char*) * new_capacity);
            if (input_filenames == NULL) {
                fatal("Out of memory.");
            }
            input_filenames_capacity = new_capacity;
        }
        input_filenames[input_filenames_count++] = *argv;
        ++argv;
    }

    // we should have found an output file.
    if (output_filename == NULL) {
        fatal("No -o output file specified.");
    }
}

static void open_output_files(void) {
    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fatal("Failed to open output file.");
    }

    chmod(output_filename, 0755);

    if (wrap_header != NULL) {
        FILE* header = fopen(wrap_header, "r");
        if (header == NULL) {
            fatal("Failed to open wrap header file.");
        }

        size_t buffer_size = 128;
        char* buffer = calloc(1, buffer_size);
        if (buffer == NULL) {
            fatal("Out of memory.");
        }
        // TODO check for errors, loop, etc.
        fread(buffer, 1, buffer_size, header);
        fclose(header);
        fwrite(buffer, 1, buffer_size, output_file);
        free(buffer);
    }

    if (option_debug) {
        char* debug_filename = 0;
        if (-1 == asprintf(&debug_filename, "%s.od", output_filename)) {
            fatal("Out of memory.");
        }
        debug_file = fopen(debug_filename, "wb");
        if (output_file == NULL) {
            fatal("Failed to open debug file.");
        }
        free(debug_filename);

        fputs("; Onramp debug info for: ", debug_file);
        fputs(output_filename, debug_file);
        fputc('\n', debug_file);
    }
}

int main(int argc, const char** argv) {
    strings_init();
    emit_init();
    symbols_init();
    labels_init();

    buffer = malloc(BUFFER_SIZE);

    parse_args(argv);

    // first pass: collect all symbol names and measure sizes.
    pass = 0;
    perform_pass(input_filenames, input_filenames_count);

    if (optimize) {
        // second pass: collect symbol usage information.
        pass = 1;
        perform_pass(input_filenames, input_filenames_count);

        // walk from the roots to mark used symbols
        symbols_walk_use();
    }

    symbols_assign_addresses();
    open_output_files();

    // third and fourth passes: collect labels and output symbols.
    pass = 2;
    perform_pass(input_filenames, input_filenames_count);

    set_current_filename(NULL);
    free(buffer);

    labels_destroy();
    symbols_destroy();
    emit_destroy();
    strings_destroy();

    ;//printf("closing output\n");
    if (debug_file) {
        fclose(debug_file);
    }
    fclose(output_file);

    //printf("total %i\n",total_labels);
    return EXIT_SUCCESS;
}
