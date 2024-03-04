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
 * This is the Onramp final assembler.
 *
 * It supports all Onramp assembler features; contains much more advanced error
 * checking than previous stages (such as checking instruction alignment,
 * argument bounds, etc.); propagates debug info correctly; and gives proper
 * error messages with line numbers.
 */

#include <string.h>

#include "common.h"
#include "emit.h"
#include "parse.h"
#include "opcodes.h"

int main(int argc, const char** argv) {

    // Parse arguments
    const char* usage = "Incorrect arguments.\n"
            "Usage: <as> <input> -o <output>";
    if (argc != 4) {
        fatal(usage);
    }
    const char* input_filename;
    const char* output_filename;
    if (0 == strcmp(argv[1], "-o")) {
        output_filename = argv[2];
        input_filename = argv[3];
    } else if (0 == strcmp(argv[2], "-o")) {
        input_filename = argv[1];
        output_filename = argv[3];
    } else {
        fatal(usage);
    }

    // Open files
    input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        fatal("Failed to open input file.");
    }
    read_char();
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        fatal("Failed to open output file.");
    }

    // Prepare
    opcodes_init();
    set_current_filename(input_filename);
    emit_line_directive(1, input_filename);
    current_line = 1;

    // Run parse loop
    while (parse()) {}

    // Clean up
    fclose(output_file);
    fclose(input_file);
    set_current_filename(NULL);
    opcodes_destroy();

    return 0;
}
